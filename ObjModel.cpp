#include <GL/glut.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include "ObjModel.hpp"

static FaceIndex parseFaceToken(const std::string& token)
{
    FaceIndex idx;
    idx.v = -1;
    idx.vt = -1;
    idx.vn = -1;

    std::stringstream ss(token);
    std::string part;
    std::vector<std::string> parts;

    while (std::getline(ss, part, '/'))
    {
        parts.push_back(part);
    }

    if (parts.size() >= 1 && !parts[0].empty())
        idx.v = atoi(parts[0].c_str()) - 1;

    if (parts.size() >= 2 && !parts[1].empty())
        idx.vt = atoi(parts[1].c_str()) - 1;

    if (parts.size() >= 3 && !parts[2].empty())
        idx.vn = atoi(parts[2].c_str()) - 1;

    return idx;
}

ObjModel::ObjModel()
    : displayListId(0)
    , hasDisplayList(false)
{
}

void ObjModel::clear()
{
    vertices.clear();
    normals.clear();
    texCoords.clear();
    faces.clear();
    if (hasDisplayList)
    {
        glDeleteLists(displayListId, 1);
        displayListId = 0;
        hasDisplayList = false;
    }
}

bool ObjModel::loadFromObjText(const std::string& filePath)
{
    clear();

    std::ifstream fin(filePath.c_str());
    if (!fin.is_open())
    {
        std::cerr << "Failed to open model file: " << filePath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(fin, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v")
        {
            Vec3 v;
            ss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        }
        else if (prefix == "vn")
        {
            Vec3 n;
            ss >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }
        else if (prefix == "vt")
        {
            Vec2 t;
            ss >> t.u >> t.v;
            texCoords.push_back(t);
        }
        else if (prefix == "f")
        {
            std::string t1, t2, t3;
            ss >> t1 >> t2 >> t3;

            if (!t1.empty() && !t2.empty() && !t3.empty())
            {
                TriangleFace tri;
                tri.a = parseFaceToken(t1);
                tri.b = parseFaceToken(t2);
                tri.c = parseFaceToken(t3);
                faces.push_back(tri);
            }
        }
    }

    fin.close();


    extern void updateLoadingProgress(const std::string& action, const std::string& itemName);
    updateLoadingProgress("Loading Model", filePath);

    return !vertices.empty() && !faces.empty();
}

void ObjModel::draw() const
{
    if (!hasDisplayList)
    {
        displayListId = glGenLists(1);
        glNewList(displayListId, GL_COMPILE);

        glBegin(GL_TRIANGLES);

        for (size_t i = 0; i < faces.size(); ++i)
        {
            const TriangleFace& f = faces[i];
            const FaceIndex idx[3] = { f.a, f.b, f.c };

            for (int k = 0; k < 3; ++k)
            {
                if (idx[k].vn >= 0 && idx[k].vn < (int)normals.size())
                {
                    const Vec3& n = normals[idx[k].vn];
                    glNormal3f(n.x, n.y, n.z);
                }

                if (idx[k].vt >= 0 && idx[k].vt < (int)texCoords.size())
                {
                    const Vec2& t = texCoords[idx[k].vt];
                    glTexCoord2f(t.u, t.v);
                }

                if (idx[k].v >= 0 && idx[k].v < (int)vertices.size())
                {
                    const Vec3& v = vertices[idx[k].v];
                    glVertex3f(v.x, v.y, v.z);
                }
            }
        }

        glEnd();
        glEndList();
        hasDisplayList = true;
    }

    glCallList(displayListId);
}

Vec3 ObjModel::getCenter() const
{
    Vec3 center = {0.0f, 0.0f, 0.0f};
    if (vertices.empty()) return center;
    float minX = vertices[0].x, maxX = vertices[0].x;
    float minY = vertices[0].y, maxY = vertices[0].y;
    float minZ = vertices[0].z, maxZ = vertices[0].z;
    for (const auto& v : vertices)
    {
        if (v.x < minX) minX = v.x;
        if (v.x > maxX) maxX = v.x;
        if (v.y < minY) minY = v.y;
        if (v.y > maxY) maxY = v.y;
        if (v.z < minZ) minZ = v.z;
        if (v.z > maxZ) maxZ = v.z;
    }
    center.x = (minX + maxX) / 2.0f;
    center.y = (minY + maxY) / 2.0f;
    center.z = (minZ + maxZ) / 2.0f;
    return center;
}

void ObjModel::getBounds(Vec3& minB, Vec3& maxB) const
{
    if (vertices.empty())
    {
        minB = {0.0f, 0.0f, 0.0f};
        maxB = {0.0f, 0.0f, 0.0f};
        return;
    }
    minB = vertices[0];
    maxB = vertices[0];
    for (const auto& v : vertices)
    {
        if (v.x < minB.x) minB.x = v.x;
        if (v.x > maxB.x) maxB.x = v.x;
        if (v.y < minB.y) minB.y = v.y;
        if (v.y > maxB.y) maxB.y = v.y;
        if (v.z < minB.z) minB.z = v.z;
        if (v.z > maxB.z) maxB.z = v.z;
    }
}
