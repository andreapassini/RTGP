#include "VertexArray.h"
#include "Renderer.h"
#include "VertexBufferLayout.h"

VertexArray::VertexArray()
{
	GlCall(glGenVertexArrays(1, &m_RendererID));
}

VertexArray::~VertexArray()
{
	GlCall(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
{
	Bind();
	vb.Bind();
	const auto& elements = layout.GetElements();
	unsigned int offset = 0;
	for (unsigned int i = 0; i < elements.size(); i++) {
		const auto& element = elements[i];
		// BUFFER LAYOUT
		GlCall(glEnableVertexAttribArray(i));
		GlCall(glVertexAttribPointer(i, element.count, element.type, 
			element.normalized, layout.GetStride(), (const void*)offset)); // Link the buffer (the last one) to the vao in vao[0]
		offset += element.count * VertexBufferElement::GetSizeOfType(element.type);
	}
}

void VertexArray::Bind() const
{
	GlCall(glBindVertexArray(m_RendererID));
}

void VertexArray::Unbind() const
{
	GlCall(glBindVertexArray(0));
}
