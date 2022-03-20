#include "ttf2mesh.h"
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

// 并查集，将同一笔中的点聚集在一起
#define MAXN 5005
int fa[MAXN], rank[MAXN];
inline void init(int n)
{
    for (int i = 0; i <= n; ++i)
    {
        fa[i] = i;
        rank[i] = 1;
    }
}
int find(int x)
{
    return x == fa[x] ? x : (fa[x] = find(fa[x]));
}
void merge(int i, int j)
{
    int x = find(i), y = find(j);
    if (rank[x] <= rank[y])
        fa[x] = y;
    else
        fa[y] = x;
    if (rank[x] == rank[y] && x != y)
        rank[y]++;
}

struct StrokeNode {
    int npoint;
    int vpoints[MAXN];
};
struct ChineseCharacter {
    int nstroke;
    StrokeNode strokes[50];
};

ChineseCharacter split_mesh_stroke(ttf_mesh_t *output)
{
    init(output->nvert);
    //printf("nface start\n");
    for (int i = 0; i < output->nfaces; ++i)
    {
        merge(output->faces[i].v1, output->faces[i].v2);
        merge(output->faces[i].v1, output->faces[i].v3);
        merge(output->faces[i].v2, output->faces[i].v3);

        merge(output->faces[i].v2, output->faces[i].v1);
        merge(output->faces[i].v3, output->faces[i].v1);
        merge(output->faces[i].v3, output->faces[i].v2);
        // printf("nface %d %d %d\n", output->faces[i].v1, output->faces[i].v2, output->faces[i].v3);
    }
    // printf("nface end\n");
    ChineseCharacter chinese;
    memset(&chinese, sizeof(chinese), 0);
    chinese.nstroke = 1;
    chinese.strokes[0].npoint = 1;
    chinese.strokes[0].vpoints[0] = 0;
    for (int i=1; i<output->nvert; i++) {
        int k = 0;
        for (k=0; k<chinese.nstroke; k++) {
            int pi = find(i);
            int pk = find(chinese.strokes[k].vpoints[0]);
            if (pi == pk) {
                chinese.strokes[k].vpoints[chinese.strokes[k].npoint] = i;
                chinese.strokes[k].npoint++;
                break;
            }
        }
        if (k>=chinese.nstroke){
            chinese.strokes[chinese.nstroke].npoint = 1;
            chinese.strokes[chinese.nstroke].vpoints[0] = i;
            chinese.nstroke++;
        }
    }

    //for (int k=0; k<chinese.nstroke; k++) {
        // printf("ci=%d ", k);
        //for (int i=0; i<chinese.strokes[k].npoint; i++)  printf("%d ", chinese.strokes[k].vpoints[i]);
        // printf("\n");
    //}
    return chinese;
}

float caculate_point_value(const float& x, const float& y) {
    return x+y;
}

float caculate_stroke_value(int v1, int v2, int v3, ChineseCharacter* pch, ttf_mesh_t *output) {
    float score = 0.0;
    const float stroke_step = 10000.0;
    float stroke_score = 0.0;
    for (int i=0; i<pch->nstroke; i++) {
        for (int j=0; j<pch->strokes[i].npoint; j++) {
            if(v1 == pch->strokes[i].vpoints[j]) score += stroke_score;
            if(v2 == pch->strokes[i].vpoints[j]) score += stroke_score;
            if(v3 == pch->strokes[i].vpoints[j]) score += stroke_score;
        }
        stroke_score += stroke_step;
    }
    score += caculate_point_value(output->vert[v1].x, output->vert[v1].y);
    score += caculate_point_value(output->vert[v2].x, output->vert[v2].y);
    score += caculate_point_value(output->vert[v3].x, output->vert[v3].y);
    return score;
}
void swap(int& a, int& b) {int iTmp=a; a=b; b=iTmp;}
void swap(StrokeNode& a, StrokeNode& b) {
    int m = a.npoint;
    if (b.npoint > m) m = b.npoint;
    swap(a.npoint, b.npoint);
    for (int i=0; i<m; i++) {
        swap(a.vpoints[i], b.vpoints[i]);
    }
}
void chinese_stroke_resort(ChineseCharacter* pch) {
    swap(pch->strokes[0], pch->strokes[2]);
}
bool isInStroke(const int v, const StrokeNode& stk) {
    for (int i=0; i<stk.npoint; i++) {
        if (v == stk.vpoints[i]) return true;
    }
    return false;
}
int mesh_vertext_resort(ttf_mesh_t *output) {
    ChineseCharacter chinese = split_mesh_stroke(output);
    chinese_stroke_resort(&chinese);
    int n = 0;
    for (int i=0; i<output->nfaces; i++) {
        for (int j=0; j<output->nfaces - i -1; j++) {
            float vp = caculate_stroke_value(output->faces[j].v1, output->faces[j].v2, output->faces[j].v3, &chinese, output);
            float vq = caculate_stroke_value(output->faces[j+1].v1, output->faces[j+1].v2, output->faces[j+1].v3, &chinese, output);
            if(vp < vq) {
                swap(output->faces[j].v1, output->faces[j+1].v1);
                swap(output->faces[j].v2, output->faces[j+1].v2);
                swap(output->faces[j].v3, output->faces[j+1].v3);
            }
        }
        // if (!isInStroke(output->faces[i].v1, chinese.strokes[0])) {
        //     output->faces[i].v1 = 0;
        //     output->faces[i].v2 = 0;
        //     output->faces[i].v3 = 0;
        // }
    }
    return chinese.nstroke;
}
