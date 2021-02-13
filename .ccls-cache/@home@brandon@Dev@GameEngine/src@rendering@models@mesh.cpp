#include <rendering/models/mesh.h>

namespace Brick3D
{
Mesh::Mesh()
{
}

Mesh::~Mesh()
{
    delete vao;
    delete ibo;
    delete vbo;
}

void Mesh::Init(const std::vector<glm::vec3> &p_positions, const std::vector<glm::vec2> &p_uvs,
                const std::vector<glm::vec3> &p_normals, const std::vector<unsigned short> &p_indices)
{
    this->m_vertices.clear();
    this->m_indices = p_indices;
    for (int i = 0; i < p_positions.size(); i++)
    {
        this->m_vertices.push_back(Vertex(p_positions.at(i), p_normals.at(i), p_uvs.at(i)));
    }

    vao = new VertexArray();
    vao->Bind();
    vao->Setup<Vertex>();
    ibo = new IndexBuffer(&m_indices[0], m_indices.size());
    vbo = new VertexBuffer(&m_vertices[0], m_vertices.size());
}

} // namespace Brick3D