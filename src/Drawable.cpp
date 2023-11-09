#include <precomp.h>

#include <Graphics/Drawable/Drawable.h>

namespace KDE
{
    void Drawable::AddBind(std::unique_ptr<Bindable> bind)
    {
        if(typeid(*bind.get()) == typeid(IndexBuffer))
        {
            assert( "Index Buffer is already bound" && m_IndexBuffer == nullptr );
            m_IndexBuffer = (IndexBuffer*)bind.get();
        }

        m_Binds.push_back( std::move(bind) );
    }

    void Drawable::Draw(KDRenderer& renderer)
    {
        for(auto& b : m_Binds)
        {
            b->Bind(renderer);
        }
        renderer.DrawIndexed(m_IndexBuffer->GetCount());
    }

    Drawable::~Drawable()
    {
        m_Binds.clear();
    }
}