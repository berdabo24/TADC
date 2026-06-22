#ifndef OBJMODEL_HPP
#define OBJMODEL_HPP

#include <vector>
#include <string>

struct Vec3
{
    float x, y, z;
};

struct Vec2
{
    float u, v;
};

struct FaceIndex
{
    int v;
    int vt;
    int vn;
};

struct TriangleFace
{
    FaceIndex a, b, c;
};

class ObjModel
{
private:
    std::vector<Vec3> vertices;
    std::vector<Vec3> normals;
    std::vector<Vec2> texCoords;
    std::vector<TriangleFace> faces;
    mutable unsigned int displayListId;
    mutable bool hasDisplayList;

public:
    ObjModel();
    void clear();
    bool loadFromObjText(const std::string& filePath);
    void draw() const;
    Vec3 getCenter() const;
    void getBounds(Vec3& minB, Vec3& maxB) const;
};

#endif
