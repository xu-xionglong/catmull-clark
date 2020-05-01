#include <vector>
#include <array>
#include <map>
#include <iostream>

using namespace std;

struct Point
{
    Point() {
        x = 0;
        y = 0;
        z = 0;
    }
    Point(float px, float py, float pz) {
        x = px;
        y = py;
        z = pz;
    }
    Point& operator += (const Point& that) {
        this->x += that.x;
        this->y += that.y;
        this->z += that.z;
        return *this;
    }
    Point& operator /= (float divider) {
        this->x /= divider;
        this->y /= divider;
        this->z /= divider;
        return *this;
    }

    float x;
    float y;
    float z;
};
Point operator + (const Point& a, const Point& b) {
    Point result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}
Point operator * (const Point& a, float multiplier) {
    Point result;
    result.x = a.x * multiplier;
    result.y = a.y * multiplier;
    result.z = a.z * multiplier;
    return result;
}

struct Edge
{
    vector<unsigned short> adjacentFaces;
    unsigned short newEdgePoint;
    Point twiceMidPoint;
};

typedef map<pair<unsigned short, unsigned short>, Edge>::iterator EdgeIterator;

struct Face
{
    vector<unsigned short> points;
    vector<EdgeIterator> edges;
    unsigned short newFacePoint;
};



struct Vertex
{
    vector<unsigned short> adjacentFaces;
    vector<EdgeIterator> adjacentEdges;
    unsigned short newVertexPoint;
};

int main(int argc, const char * argv[]) {
    vector<Point> vertexData = {
        {1.000000, -1.000000, -1.000000},
        {1.000000, -1.000000, 1.000000},
        {-1.000000, -1.000000, 1.000000},
        {-1.000000, -1.000000, -1.000000},
        {1.000000, 1.000000, -1.000000},
        {1.000000, 1.000000, 1.000000},
        {-1.000000, 1.000000, 1.000000},
        {-1.000000, 1.000000, -1.000000},
    };
    
    vector<vector<unsigned short>> indexData = {
        {0, 1, 2, 3},
        {4, 7, 6, 5},
        {0, 4, 5, 1},
        {1, 5, 6, 2},
        {2, 6, 7, 3},
        {4, 0, 3, 7},
    };
    
    vector<Point> newVertexData;
    vector<vector<unsigned short>> newIndexData;
    
    vector<Face> faces;
    vector<Vertex> vertices;
    map<pair<unsigned short, unsigned short>, Edge> edges;
    
    vertices.resize(vertexData.size());
    for (int i = 0; i < indexData.size(); ++ i) {
        Face face;
        Point newFacePoint;
        face.points = indexData[i];
        size_t indexCount = face.points.size();
        for (int j = 0; j < indexCount; ++ j) {
            unsigned short vertexIndex = face.points[j];
            Vertex& vertex = vertices[vertexIndex];
            vertex.adjacentFaces.push_back(i);
            //new face points - the average of all of the old points defining the face
            
            newFacePoint += vertexData[vertexIndex];
            
            
            unsigned short nextVertexIndex = face.points[(j + 1) == indexCount ? 0 : j + 1];
            if (nextVertexIndex < vertexIndex) {
                swap(vertexIndex, nextVertexIndex);
            }
            
            auto edgeKey = make_pair(vertexIndex, nextVertexIndex);
            auto iter = edges.find(edgeKey);
            if (iter == edges.end()) {
                iter = edges.insert(make_pair(edgeKey, Edge())).first;
            }
            iter->second.adjacentFaces.push_back(i);
            vertex.adjacentEdges.push_back(iter);
            face.edges.push_back(iter);
        }
        newFacePoint /= face.points.size();
        face.newFacePoint = (unsigned short)(newVertexData.size());
        newVertexData.push_back(newFacePoint);
        faces.push_back(move(face));
    }
    
    for (auto iter = edges.begin(); iter != edges.end(); ++ iter) {
        //new edge points - the average of the midpoints of the old edg with
        //the average of the two new face points of the faces sharing the edge
        Point sum = vertexData[iter->first.first] + vertexData[iter->first.second];
        iter->second.twiceMidPoint = sum;
        int count = 2;
        int faceCount = int(iter->second.adjacentFaces.size());
        count += faceCount;
        for (int i = 0; i < faceCount; ++ i) {
            sum += newVertexData[faces[iter->second.adjacentFaces[i]].newFacePoint];
        }
        sum /= count;
        
        iter->second.newEdgePoint = (unsigned short)(newVertexData.size());
        newVertexData.push_back(sum);
    }
    
    
    for (int i = 0; i < vertexData.size(); ++ i) {
        //(Q / n) + (2R / n) + (S(n - 3) / n)
        int n = int(vertices[i].adjacentFaces.size());
        Point sum;
        for (int j = 0; j < n; ++ j) {
            //the average of the new face points of all faces adjacent to the old vertex point
            sum += newVertexData[faces[vertices[i].adjacentFaces[j]].newFacePoint];
            
            //the average of midpoints of all old edges incident on the old vertex point
            //num of edge should equals to num of faces
            sum += vertices[i].adjacentEdges[j]->second.twiceMidPoint;
        }
        sum /= n;
        
        //old vertex point
        sum += vertexData[i] * (n - 3);
        
        sum /= n;
        
        vertices[i].newVertexPoint = (unsigned short)(newVertexData.size());
        newVertexData.push_back(sum);
    }
    
    for (int i = 0; i < faces.size(); ++ i) {
        Face& face = faces[i];
        for(int j = 0; j < faces[i].points.size(); ++ j) {
            
            vector<unsigned short> indices;
            indices.reserve(4);
            indices.push_back(vertices[face.points[j]].newVertexPoint);
            indices.push_back(face.edges[j]->second.newEdgePoint);
            indices.push_back(face.newFacePoint);
            indices.push_back(face.edges[j > 0 ? j - 1 : face.edges.size() - 1]->second.newEdgePoint);
            
            newIndexData.push_back(indices);
        }
    }
    
    for (int i = 0; i < newVertexData.size(); ++ i) {
        cout<<"v "<<newVertexData[i].x<<" "<<newVertexData[i].y<<" "<<newVertexData[i].z<<endl;
    }
    for (int i = 0; i < newIndexData.size(); ++ i) {
        cout<<"f";
        for (int j = 0; j < newIndexData[i].size(); ++ j) {
            cout<<" "<<newIndexData[i][j] + 1<<"//"<<i + 1;
        }
        cout<<endl;
    }
}
