#include "vertexArray.hpp"

VertexArray::VertexArray() {
	glGenVertexArrays(1, &arrayID);
}

VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &arrayID);
}

void VertexArray::addBuffer(const VertexBuffer & vb, const VertexBufferLayout & layout, unsigned int index) {
	bind();
	vb.bind();
	const auto& elements = layout.getElements();
	unsigned int offset = 0;
	for (unsigned int i = index; i < elements.size() + index; ++i) {
		const auto& element = elements[i - index];
		glEnableVertexAttribArray(i);
		if (element.type == GL_INT) {
			glVertexAttribIPointer(i, element.count, element.type, layout.getStride(), (const void*)offset);
		} else {
			glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.getStride(), (const void*)offset);
		}
		offset += element.count * VertexBufferLayoutElement::getSizeOfType(element.type);
	}
}

void VertexArray::bind() const {
	glBindVertexArray(arrayID);
}

void VertexArray::unbind() const {
	glBindVertexArray(0);
}