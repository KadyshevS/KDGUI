#include <precomp.h>

#include <WinBase/KDWindow.h>

#include <Debug/KDException.h>
#include <Debug/WndExcept.h>

#include <Input/Input.h>

#include <Events/KDEvent.h>
#include <Events/ApplicationEvent.h>
#include <Events/MouseEvent.h>
#include <Events/KeyEvent.h>

struct KDWindowData_win32
{
    bool IsShouldClose = false;
    bool IsCursorEnabled = true;
    int Width = 0, Height = 0;
    std::string Title;
    std::function<void(KDE::KDEvent&)> EventFunc = nullptr;
    std::vector<BYTE> RawBuffer;

    HINSTANCE hInstance = NULL;
    HWND hWnd = NULL;
    MSG msg = {};
};

LRESULT HandleMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    KDE::KDWindow* const pWnd = (KDE::KDWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    KDWindowData_win32* const pWndData = (KDWindowData_win32*)pWnd->GetNativeData();
    static bool isCursorInWindow = false;  

    KDE::MouseScrolledEvent eDeltaRefresh(0, 0);
    if(pWndData->EventFunc != nullptr)
        pWndData->EventFunc(eDeltaRefresh);

    switch(msg)
    {
        case WM_CREATE:
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_CLOSE:
            {
                KDE::WindowCloseEvent e;
                pWndData->EventFunc(e);
                PostQuitMessage(0);
                break;
            }
        case WM_KEYDOWN:
            {
                KDE::KeyPressedEvent e((int)wparam, (int)lparam);
                pWndData->EventFunc(e);
                break;
            }
        case WM_KEYUP:
            {
                KDE::KeyReleasedEvent e((int)wparam);
                pWndData->EventFunc(e);
                break;
            }
        case WM_SYSKEYDOWN:
            {
                KDE::KeyPressedEvent e((int)wparam, (int)lparam);
                pWndData->EventFunc(e);
                break;
            }
        case WM_SYSKEYUP:
            {
                KDE::KeyReleasedEvent e((int)wparam);
                pWndData->EventFunc(e);
                break;
            }
        case WM_CHAR:
            {
                KDE::KeyCharEvent e((int)wparam, (int)lparam);
                pWndData->EventFunc(e);
                break;
            }
        case WM_LBUTTONDOWN:
            {
                KDE::MouseButtonPressedEvent e(KDE::Mouse::LeftMouseBtn);
                pWndData->EventFunc(e);
                break;
            }
        case WM_RBUTTONDOWN:
            {
                KDE::MouseButtonPressedEvent e(KDE::Mouse::RightMouseBtn);
                pWndData->EventFunc(e);
                break;
            }
        case WM_LBUTTONUP:
            {
                KDE::MouseButtonReleasedEvent e(KDE::Mouse::LeftMouseBtn);
                pWndData->EventFunc(e);
                break;
            }
        case WM_RBUTTONUP:
            {
                KDE::MouseButtonReleasedEvent e(KDE::Mouse::RightMouseBtn);
                pWndData->EventFunc(e);
                break;
            }
        case WM_MOUSEWHEEL:
            {
                int zDelta = (int)GET_WHEEL_DELTA_WPARAM(wparam) / 120;
                KDE::MouseScrolledEvent e(0, zDelta);
                pWndData->EventFunc(e);
                break;
            }
        case WM_INPUT:
            {
                UINT size = 0;

                if( GetRawInputData(
                    (HRAWINPUT)lparam, 
                    RID_INPUT, nullptr, 
                    &size, sizeof(RAWINPUTHEADER)) == -1 )
                {
                    break;
                }

                pWndData->RawBuffer.resize(size);

                if ( GetRawInputData(
                    (HRAWINPUT)lparam,
                    RID_INPUT, pWndData->RawBuffer.data(),
                    &size, sizeof(RAWINPUTHEADER)) != size )
                {
                    break;
                }

                auto& ri = reinterpret_cast<const RAWINPUT&>(*pWndData->RawBuffer.data());
                if (ri.header.dwType == RIM_TYPEMOUSE &&
                    (ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0))
                {
                    KDE::MouseRawEvent e(
                        (int)ri.data.mouse.lLastX, 
                        (int)ri.data.mouse.lLastY);
                    pWndData->EventFunc(e);
                }
                break;
            }
        case WM_MOUSEMOVE:
            {
                const POINTS pt = MAKEPOINTS(lparam);      
                if(pt.x >= 0 && pt.x < pWndData->Width && pt.y >= 0 && pt.y < pWndData->Height)
                {
                    if(!isCursorInWindow)
                    {
                        SetCapture(hwnd);
                        KDE::MouseEnterEvent e;
                        pWndData->EventFunc(e);    
                        isCursorInWindow = true;
                    }
                    KDE::MouseMovedEvent e(pt.x, pt.y);
                    pWndData->EventFunc(e);
                }
                else
                {
                    if( wparam & (MK_LBUTTON | MK_RBUTTON) )
                    {
                        KDE::MouseMovedEvent e(pt.x, pt.y);
                        pWndData->EventFunc(e);
                    }
                    else
                    {
                        ReleaseCapture();
                        KDE::MouseLeaveEvent e;
                        pWndData->EventFunc(e);    
                        isCursorInWindow = false;
                    }
                }
                break;
            }
        case WM_SIZE:
            {
                KDE::WindowResizeEvent e(LOWORD(lparam), HIWORD(lparam));
                if(pWndData->EventFunc != nullptr)
                    pWndData->EventFunc(e);
                break;
            }
        case WM_MOVE:
            {
                KDE::WindowMovedEvent e(LOWORD(lparam), HIWORD(lparam));
                if(pWndData->EventFunc != nullptr)
                    pWndData->EventFunc(e);
                break;
            }
        case WM_SETFOCUS:
            {
                KDE::WindowFocusEvent e;
                if(pWndData->EventFunc != nullptr)
                    pWndData->EventFunc(e);
                break;
            }
        case WM_KILLFOCUS:
            {
                KDE::WindowLostFocusEvent e;
                pWndData->EventFunc(e);
                break;
            }
        default:
            return DefWindowProc(hwnd, msg, wparam, lparam); 
    }      
    return NULL;
}
static LRESULT CALLBACK HandleMsgThunk(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	return HandleMsg(hwnd, msg, wparam, lparam);
}
static LRESULT CALLBACK HandleMsgSetup(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if( msg == WM_NCCREATE )
	{
		const CREATESTRUCTW* const pCreate = (CREATESTRUCTW*)lparam;
		KDE::KDWindow* const pWnd = (KDE::KDWindow*)pCreate->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pWnd);
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)&HandleMsgThunk);
	    return HandleMsg(hwnd, msg, wparam, lparam);
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

namespace KDE
{
    KDWindow::KDWindow(int width, int height, const char *title)
    {
        m_Data = new KDWindowData_win32{};
        KDWindowData_win32* riData = (KDWindowData_win32*)m_Data;
        
        riData->Width = width;
        riData->Height = height;
        riData->Title = title;
        riData->hInstance = GetModuleHandle(NULL);

        WNDCLASSEX wc{};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = HandleMsgSetup;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = riData->hInstance;
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
        wc.lpszMenuName = nullptr;
        wc.lpszClassName = "KDWindowTestClass1";
        RegisterClassEx(&wc);

        riData->hWnd = CreateWindowEx(
            WS_EX_OVERLAPPEDWINDOW, "KDWindowTestClass1", riData->Title.c_str(),
            WS_OVERLAPPEDWINDOW | WS_MINIMIZEBOX | WS_SYSMENU, 20, 20, 
            width, height, NULL, NULL, riData->hInstance, this
        );

        if(riData->hWnd == NULL)
            throw KDWND_LAST_EXCEPT;

        RECT clientRect = { 0, 0, width, height };
        AdjustWindowRect(&clientRect, 
            WS_OVERLAPPEDWINDOW | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

        SetWindowPos(riData->hWnd, NULL, 20, 20,
            clientRect.right - clientRect.left + 4, 
            clientRect.bottom - clientRect.top + 4, 0);

        ShowWindow(riData->hWnd, SW_SHOW);

        RAWINPUTDEVICE rid{};
        rid.usUsagePage = 0x01;
        rid.usUsage = 0x02;
        rid.dwFlags = 0;
        rid.hwndTarget = nullptr;
        if ( RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE )
            throw KDWND_LAST_EXCEPT;
    }
    KDWindow::~KDWindow()
    {
        UnregisterClass("KDWindowTestClass1", ((KDWindowData_win32*)m_Data)->hInstance);
        DestroyWindow(((KDWindowData_win32*)m_Data)->hWnd);
        delete (KDWindowData_win32*)m_Data;
    }

    void KDWindow::SetEventFunc(std::function<void(KDEvent&)> eventFunc)
    {
        ( (KDWindowData_win32*)m_Data )->EventFunc = eventFunc;
    }
    void KDWindow::Update()
    {
        KDWindowData_win32* riData = (KDWindowData_win32*)m_Data;

        KDE::AppUpdateEvent eUpd;
        riData->EventFunc(eUpd);

        while(PeekMessage(&riData->msg, NULL, 0, 0, PM_REMOVE))
        {
            if(riData->msg.message == WM_QUIT)
            {
                riData->IsShouldClose = true;
                break;
            }

            TranslateMessage(&riData->msg);
            DispatchMessage(&riData->msg);
        }
    }
    bool KDWindow::ShouldClose()
    {
        return ( (KDWindowData_win32*)m_Data )->IsShouldClose;
    }

    void KDWindow::ShowCursor()
    {
        ( (KDWindowData_win32*)m_Data )->IsCursorEnabled = true;
        while (::ShowCursor(TRUE), 0);
    }
    void KDWindow::HideCursor()
    {
        ( (KDWindowData_win32*)m_Data )->IsCursorEnabled = false;
        SetCursorPos(
            ( (KDWindowData_win32*)m_Data )->Width / 2,
            ( (KDWindowData_win32*)m_Data )->Height / 2
        );
        while (::ShowCursor(FALSE) >= 0);
    }
    bool KDWindow::IsCursorEnabled() const
    {
        return ( (KDWindowData_win32*)m_Data )->IsCursorEnabled;
    }

    int KDWindow::GetWidth() const
    {
        return ( (KDWindowData_win32*)m_Data )->Width;
    }
    int KDWindow::GetHeight() const
    {
        return ( (KDWindowData_win32*)m_Data )->Height;
    }

    void KDWindow::ShowMessageBox(const char* text, const char* caption)
    {
        MessageBox(NULL, text, caption, MB_ICONEXCLAMATION | MB_OK);
    }
}