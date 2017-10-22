#pragma once

#include <stdlib.h>
#include <assert.h>
#include <vector>
#include <GL/glew.h>


class BufferRender
{
protected:
	unsigned int  m_VAO;
	unsigned int  m_vertexBuffer;
	unsigned int  m_indiceBuffer;
	//GLuint  m_textureBuffer;
	std::vector<unsigned int>  m_elementVertexIndexs;
	std::vector<unsigned int>  m_elementVertexCounts;
	std::vector<unsigned int>  m_elementIndiceIndexs;
	std::vector<unsigned int>  m_elementIndiceCounts;

public:
	BufferRender(const std::string &filename);
	~BufferRender() { deletGL(); }

	void  deletGL() {
		if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
		if (m_vertexBuffer) glDeleteVertexArrays(1, &m_vertexBuffer);
		if (m_indiceBuffer) glDeleteVertexArrays(1, &m_indiceBuffer);
	}
	void  render();
};
