
#include "modeler.h"
#include "proto\autoVR.pb.h"
#include "io.h"

enum BUFFER_ATTRIBUTE_INDEX
{
	BUFFER_VERTEX_INDEX = 0,
	BUFFER_NORMAL_INDEX,
	BUFFER_TANGENT_INDEX,
	BUFFER_TEXTURE_INDEX,
	BUFFER_ATTRIBUTE_INDEX_NUM
};

#define BUFFER_VERTEX_ATTRIBUTE_SIZE  3
#define BUFFER_NORMAL_ATTRIBUTE_SIZE  3
#define BUFFER_TANGENT_ATTRIBUTE_SIZE 3
#define BUFFER_TEXTURE_ATTRIBUTE_SIZE 2

#define MAX_STROKE_VERTICE_BUFFER_SIZE 800000  


inline bool
j_initialNewVAO(GLuint *newVAO, GLuint *newVertexBuffer, GLuint *newIndiceBuffer, int size, int *attributeIndexs = 0, GLuint *attributeSizes = 0, GLuint **newAttributeBuffers = 0)
{
	// Setup the VAO to format the data
	glGenVertexArrays(1, newVAO);
	if (*newVAO <= 0)
		return false;
	glBindVertexArray(*newVAO);

	//create indices buffer
	if (newIndiceBuffer) {
		for (int i = 0; i < size; i++)
			assert(!newAttributeBuffers[i]); //not multi-buffer index
		glGenBuffers(1, newIndiceBuffer);
		if (*newIndiceBuffer <= 0)
			return false;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *newIndiceBuffer);
	}

	int stride = 3;
	for (int i = 0; i < size; i++)
		if (!newAttributeBuffers[i])
			stride += attributeSizes[i];
	if (stride == 3)
		stride = 0;

	int offset = 0;

	//create vertex buffer
	glGenBuffers(1, newVertexBuffer);
	if (*newVertexBuffer <= 0)
		return false;
	glBindBuffer(GL_ARRAY_BUFFER, *newVertexBuffer);
	glEnableVertexAttribArray(BUFFER_VERTEX_INDEX);
	glVertexAttribPointer(BUFFER_VERTEX_INDEX, BUFFER_VERTEX_ATTRIBUTE_SIZE, GL_FLOAT, GL_FALSE, stride * sizeof(float), (const void *)offset);
	offset += BUFFER_VERTEX_ATTRIBUTE_SIZE * sizeof(float);

	for (int i = 0; i < size; i++) {
		if (attributeSizes[i] == 0)
			continue;
		if (newAttributeBuffers[i]) {
			glGenBuffers(1, newAttributeBuffers[i]);
			assert(*(newAttributeBuffers[i]) > 0);
			glBindBuffer(GL_ARRAY_BUFFER, *(newAttributeBuffers[i]));
			glEnableVertexAttribArray(attributeIndexs[i]);
			glVertexAttribPointer(attributeIndexs[i], attributeSizes[i], GL_FLOAT, GL_FALSE, 0, 0);
		}
		else {
			glEnableVertexAttribArray(attributeIndexs[i]);
			glVertexAttribPointer(attributeIndexs[i], attributeSizes[i], GL_FLOAT, GL_FALSE, stride * sizeof(float), (const void *)offset);
			offset += attributeSizes[i] * sizeof(float);
		}
	}

	glBindVertexArray(0);

	return true;
}


int
j_getStrokeVertices(autoVR::Operation *data, std::vector<float> &verticeData, bool *attributes)
{
	int pointSize = data->points_size();

	int count = 0;

	int sizeperv = 0;
	if (attributes[BUFFER_VERTEX_INDEX])
		sizeperv += BUFFER_VERTEX_ATTRIBUTE_SIZE;
	if (attributes[BUFFER_NORMAL_INDEX])
		sizeperv += BUFFER_NORMAL_ATTRIBUTE_SIZE;
	if (attributes[BUFFER_TANGENT_INDEX])
		sizeperv += BUFFER_TANGENT_ATTRIBUTE_SIZE;
	if (attributes[BUFFER_TEXTURE_INDEX])
		sizeperv += BUFFER_TEXTURE_ATTRIBUTE_SIZE;

	int vertexSize = data->points_size() * sizeperv; //position+color, for now
	if (verticeData.size() < vertexSize)
		verticeData.resize(vertexSize);

	autoVR::Operation_Point *pos = 0, *tangent = 0, *norm = 0;
	autoVR::Operation_Texture *texture = 0;
	for (int i = 0; i < data->points_size(); i++) {
		pos = data->mutable_points(i);
		tangent = data->mutable_pitches(i);
		norm = data->mutable_normals(i);
		texture = data->mutable_textures(i);

		if (attributes[BUFFER_VERTEX_INDEX]) {
			verticeData[count++] = pos->x();
			verticeData[count++] = pos->y();
			verticeData[count++] = pos->z();
		}
		if (attributes[BUFFER_NORMAL_INDEX]) {
			verticeData[count++] = norm->x();
			verticeData[count++] = norm->y();
			verticeData[count++] = norm->z();
		}
		if (attributes[BUFFER_TANGENT_INDEX]) {
			verticeData[count++] = tangent->x();
			verticeData[count++] = tangent->y();
			verticeData[count++] = tangent->z();
		}
		if (attributes[BUFFER_TEXTURE_INDEX]) {
			verticeData[count++] = texture->u();
			verticeData[count++] = texture->v();
		}

	}
	return count;
}


int
j_getStrokeIndices(autoVR::Operation *data, std::vector<GLuint> &indiceData, int firstId)
{
	//the stroke points are organized like this ":::::::::", 2 * len = points
	int hlineSize = 2;
	int vlineSize = data->points_size() / 2;

	firstId += hlineSize;

	int vertexSize = vlineSize * (hlineSize - 1) * 6; //
	if (indiceData.size() < vertexSize)
		indiceData.resize(vertexSize);

	int count = 0;
	for (int i = 1; i < vlineSize; i++) {
		for (int j = 0; j < hlineSize - 1; j++) {
			indiceData[count++] = firstId + j - hlineSize;
			indiceData[count++] = firstId + j + 1 - hlineSize;
			indiceData[count++] = firstId + j;

			indiceData[count++] = firstId + j + 1 - hlineSize;
			indiceData[count++] = firstId + j + 1;
			indiceData[count++] = firstId + j;
		}
		firstId += hlineSize;
	}
	return count;
}



BufferRender::BufferRender(const std::string &filename)
{
	//read
	autoVR::Painting painting;
	ReadProtoFromTextFile(filename, &painting);

	std::vector<float> vertexes, vertexes1;
	std::vector<GLuint> indices, indices1;

	int offsetId = 1;
	bool attributes[5] = { true, true, true, true };  //pos + norm + tangents + texture
	for (int i = 0; i < painting.operations_size(); i++) {
		vertexes1.clear(); indices1.clear();
		j_getStrokeVertices(painting.mutable_operations(i), vertexes1, attributes);  //get the vertex data of stroke i
		j_getStrokeIndices(painting.mutable_operations(i), indices1, offsetId); //get the indice data of stroke i
		vertexes.insert(vertexes.end(), vertexes1.begin(), vertexes1.end());
		indices.insert(indices.end(), indices1.begin(), indices1.end());
		offsetId += painting.mutable_operations(i)->points_size();
	}

	m_VAO = m_vertexBuffer = m_indiceBuffer = 0;

	int attributeIndexs[] = { BUFFER_NORMAL_INDEX, BUFFER_TANGENT_INDEX, BUFFER_TEXTURE_INDEX };
	GLuint attributeSizes[] = { BUFFER_NORMAL_ATTRIBUTE_SIZE, BUFFER_TANGENT_ATTRIBUTE_SIZE, BUFFER_TEXTURE_ATTRIBUTE_SIZE };
	GLuint *attrbuteBuffers[] = { nullptr, nullptr, nullptr };
	assert(j_initialNewVAO(&m_VAO, &m_vertexBuffer, &m_indiceBuffer, 3, attributeIndexs, attributeSizes, attrbuteBuffers));

	assert(m_VAO);
	assert(m_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertexes.size(), &vertexes[0], GL_DYNAMIC_DRAW);
	if (m_indiceBuffer) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indiceBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*indices.size(), &indices[0], GL_DYNAMIC_DRAW);
	}
}


void
BufferRender::render()
{
	//set shader uniform parameters
	//Shader *shader;
	//shader->use();
	//shader->setFloat("m_opacity", 1.0f);

	glBindVertexArray(m_VAO);
	glMultiDrawElements(GL_TRIANGLES, (GLsizei*)(&m_elementIndiceCounts[0]), GL_UNSIGNED_INT, (const void **)(&m_elementIndiceIndexs[0]), m_elementIndiceCounts.size());
	glBindVertexArray(0);
}