#include <precomp.h>

#include <Graphics/Drawable/Drawable.h>

namespace KDE
{
    void Drawable::AddBind(Bindable* bind)
    {
        if(typeid(*bind) == typeid(IndexBuffer))
            m_IndexBuffer = (IndexBuffer*)bind;

        m_Binds.push_back(bind);
    }

    void Drawable::Draw(KDRenderer& renderer)
    {
        for(auto& b : m_Binds)
        {
            b->Bind(renderer);
        }
        renderer.DrawIndexed(m_IndexBuffer->GetCount());
    }
}