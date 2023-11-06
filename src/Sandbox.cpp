#include <precomp.h>
#include <KDEngine.h>

class SandboxApp : public KDE::KDApplication
{
    public:
        void OnCreate() override
        {
            std::mt19937 rng(std::random_device{}());
            std::uniform_real_distribution<float> adist(0.0f, 3.14f * 4.0f);
            std::uniform_real_distribution<float> bdist(0.0f, 3.14f * 2.0f);
            std::uniform_real_distribution<float> cdist(0.0f, 3.14f * 0.3f);
            std::uniform_real_distribution<float> ddist(6.0f, 20.0f);

            for(int i = 0; i < 60; i++)
                texCubes.emplace_back(new KDE::TexturedCube(*renderer, rng, adist, bdist, cdist, ddist));
        }
        void OnUpdate() override
        {
            for(auto& cube : texCubes)
            {
                cube->Update(*renderer, renderer->GetDeltaTime());
                cube->Draw(*renderer);
            }
        }
        void OnDestroy() override
        {
        }
    private:
        std::vector<KDE::TexturedCube*> texCubes;
};

int main(int argc, char* argv[])
{
    try
    {
        SandboxApp* app1 = new SandboxApp;
        app1->Run();
        delete app1;
    }
    catch(const KDE::KDException& e)
    {
        KDE::KDWindow::ShowMessageBox(e.what(), e.type());
    }
    catch(const std::exception& e)
    {
        KDE::KDWindow::ShowMessageBox(e.what(), "Standard Exception");
    }
    catch(...)
    {
        KDE::KDWindow::ShowMessageBox("No details available", "Unknown Exception");
    }

    return 0;
}