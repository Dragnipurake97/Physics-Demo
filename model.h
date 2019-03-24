// GL Math Library - https://github.com/g-truc/glm
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

struct Vertex
{
	float vertex[3] = { 0, 0,0 };
	float normals[3] = { 0, 0, 0 };
};

struct Model
{
	// Mesh Data
	std::vector<Vertex> vertex;

	std::vector<float> vertices;
	std::vector<float> normals; 

	std::vector<int> faces;
	std::vector<int> texture_indices;
	std::vector<int> normal_indices;

	unsigned int vao, vbo, ebo;

	// Physics Data
	glm::vec3 velocity = { 0, 0, 0 };
	glm::mat4 position; // Matrix to move mesh to position in world space
	glm::vec3 pos = { 0, 0, 0 }; // Vector to describe position

	float rad; // radius (for spheres)

	Model()	{}

	Model(std::string filename)
	{
		loadModel(filename);

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertex.size() * (6 * sizeof(float)), vertex.data(), GL_STATIC_DRAW);

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(int), faces.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}

	void move(glm::vec3 trans)
	{
		position = glm::translate(position, trans);
		pos = pos + trans;
	}

	// Used to set the state of the ball
	void setState(float trans[3], float vel[3])
	{
		position = glm::mat4();
		glm::vec3 temp(trans[0], trans[1], trans[2]);
		position = glm::translate(position, temp);
		pos = glm::vec3(0, 0, 0) + temp;

		velocity = glm::vec3(vel[0], vel[1], vel[2]);
	}


	void loadModel(std::string filename)
	{
		std::ifstream file;
		std::string line;
		std::string temp;

		file.open(filename);
		if (file.is_open())
		{
			float v, vn, uv;
			int t, prev_t, next_t;

			while (file >> temp)
			{
				if (temp == "v")
				{
					file >> v;
					vertices.push_back(v);
					file >> v;
					vertices.push_back(v);
					file >> v;
					vertices.push_back(v);
				}
				else if (temp == "vn")
				{
					file >> vn;
					normals.push_back(v);
					file >> vn;
					normals.push_back(v);
					file >> vn;
					normals.push_back(v);
				}
				else if (temp == "f")
				{
					file >> temp;
					t = temp.find("/");
					// Get vertex index
					if (t == std::string::npos)
					{
						faces.push_back(std::stoi(temp));
					}
					else
					{
						faces.push_back(std::stoi(temp.substr(0, t)));
					}
					prev_t = t;

					// Get tex coord index
					t = temp.find("/", t + 1);
					if (t != std::string::npos && t != prev_t + 1)
					{
						texture_indices.push_back(std::stoi(temp.substr(prev_t + 1, t)));
					}

					// Get Normal index
					if (t != temp.size() - 1)
					{
						normal_indices.push_back(std::stoi(temp.substr(t + 1)));
					}
					prev_t = t;



					file >> temp;
					t = temp.find("/");
					// Get vertex index
					if (t == std::string::npos)
					{
						faces.push_back(std::stoi(temp));
					}
					else
					{
						faces.push_back(std::stoi(temp.substr(0, t)));
					}
					prev_t = t;

					// Get tex coord index
					t = temp.find("/", t + 1);
					if (t != std::string::npos && t != prev_t + 1)
					{
						texture_indices.push_back(std::stoi(temp.substr(prev_t + 1, t)));
					}

					// Get Normal index
					if (t != temp.size() - 1)
					{
						normal_indices.push_back(std::stoi(temp.substr(t + 1)));
					}
					prev_t = t;



					file >> temp;
					t = temp.find("/");
					// Get vertex index
					if (t == std::string::npos)
					{
						faces.push_back(std::stoi(temp));
					}
					else
					{
						faces.push_back(std::stoi(temp.substr(0, t)));
					}
					prev_t = t;

					// Get tex coord index
					t = temp.find("/", t + 1);
					if (t != std::string::npos && t != prev_t + 1)
					{
						texture_indices.push_back(std::stoi(temp.substr(prev_t + 1, t)));
					}

					// Get Normal index
					if (t != temp.size() - 1)
					{
						normal_indices.push_back(std::stoi(temp.substr(t + 1)));
					}
					prev_t = t;
				}
			}
		}
		else
		{
			std::cout << "Cannot open file: " << filename << std::endl;
		}
		
		Vertex v;
		int vert, normal;

		// Loop through arrays to build a Vertex structure which can be passed into a buffer
		for (int i = 0; i < faces.size(); i++)
		{
			// Get vertex
			vert = faces.at(i) - 1;
			v.vertex[0] = vertices.at(vert * 3); // x 
			v.vertex[1] = vertices.at((vert * 3) + 1); // y
			v.vertex[2] = vertices.at((vert * 3) + 2); // z

			// Get normal
			normal = normal_indices.at(i) - 1;
			v.normals[0] = normals.at(normal * 3);
			v.normals[1] = normals.at((normal * 3) + 1);
			v.normals[2] = normals.at((normal * 3) + 2);

			// Add to vector of vertices
			vertex.push_back(v);
		}

		// Get radius (any point as centre is (0, 0, 0)
		glm::vec3 vec_rad = glm::vec3(vertex.at(3).vertex[0], vertex.at(3).vertex[1], vertex.at(3).vertex[2]);
		rad = sqrt(vec_rad.x * vec_rad.x + vec_rad.y * vec_rad.y + vec_rad.z * vec_rad.z);
	}
};

Model loadModel(std::string filename)
{
	Model m;

	std::ifstream file;
	std::string line;
	std::string temp;

	file.open(filename);
	if (file.is_open())
	{
		float v, vn, uv;
		int t, prev_t, next_t;

		while (file >> temp)
		{
			if (temp == "v")
			{
				file >> v;
				m.vertices.push_back(v);
				file >> v;
				m.vertices.push_back(v);
				file >> v;
				m.vertices.push_back(v);
			}
			else if (temp == "vn")
			{
				file >> vn;
				m.normals.push_back(v);
				file >> vn;
				m.normals.push_back(v);
				file >> vn;
				m.normals.push_back(v);
			}
			else if (temp == "f")
			{
				file >> temp;
				t = temp.find("/");
				// Get vertex index
				if (t == std::string::npos)
				{
					m.faces.push_back(std::stoi(temp));
				}
				else
				{
					m.faces.push_back(std::stoi(temp.substr(0, t)));
				}
				prev_t = t;

				// Get tex coord index
				t = temp.find("/", t + 1);
				if (t != std::string::npos || t != prev_t + 1)
				{
					m.texture_indices.push_back(std::stoi(temp.substr(prev_t + 1, t)));
				}

				// Get Normal index
				if (t != temp.size() - 1)
				{
					m.normal_indices.push_back(std::stoi(temp.substr(prev_t + 1, t)));
				}
				prev_t = t;



				file >> temp;
				t = temp.find("/");
				// Get vertex index
				if (t == std::string::npos)
				{
					m.faces.push_back(std::stoi(temp));
				}
				else
				{
					m.faces.push_back(std::stoi(temp.substr(0, t)));
				}
				prev_t = t;

				// Get tex coord index
				t = temp.find("/", t + 1);
				if (t != std::string::npos || t != prev_t + 1)
				{
					m.texture_indices.push_back(std::stoi(temp.substr(prev_t + 1, t)));
				}

				// Get Normal index
				if (t != temp.size() - 1)
				{
					m.normal_indices.push_back(std::stoi(temp.substr(prev_t + 1, t)));
				}
				prev_t = t;



				file >> temp;
				t = temp.find("/");
				// Get vertex index
				if (t == std::string::npos)
				{
					m.faces.push_back(std::stoi(temp));
				}
				else
				{
					m.faces.push_back(std::stoi(temp.substr(0, t)));
				}
				prev_t = t;

				// Get tex coord index
				t = temp.find("/", t + 1);
				if (t != std::string::npos || t != prev_t + 1)
				{
					m.texture_indices.push_back(std::stoi(temp.substr(prev_t + 1, t)));
				}

				// Get Normal index
				if (t != temp.size() - 1)
				{
					m.normal_indices.push_back(std::stoi(temp.substr(prev_t + 1, t)));
				}
				prev_t = t;
			}
		}
	}
	else
		std::cout << "Cannot open file: " << filename << std::endl;


	return m;
}