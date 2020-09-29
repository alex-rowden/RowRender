#include "DataBuffer.h"
template<typename T>
DataBuffer<typename T>::DataBuffer(int num_elements) {
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);

	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * num_elements, NULL, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, counter++, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
template<typename T>
void DataBuffer<typename T>::clearBuffer() {
	//glClearBufferData(GL_ARRAY_BUFFER, )
	std::cerr << "Calling unimplemented DataBuffer::clearBuffer" << std::endl;
}

