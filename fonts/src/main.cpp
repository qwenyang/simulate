#include <GLFW/glfw3.h>
#include <windows.h>
#include "ttf2mesh.h"
#include "stroke_sort.h"
#include <stdio.h>
#include <unistd.h>
static int view_mode = 0;

static ttf_t *font = NULL;
// static ttf_glyph_t *glyph = NULL;
// static ttf_mesh_t *mesh = NULL;

static int draw_start_x = 0;
static int draw_start_y = 300;
static int mesh_array_index = 0;
static ttf_mesh_t *mesh_array[100];

static int square_length_x = 50;
static int square_length_y = 50;

static bool load_system_font()
{
    // list all system fonts by filename mask:

    /*ttf_t **list = ttf_list_system_fonts("DejaVuSans*|Ubuntu*|FreeSerif*|Arial*|Cour*");
    if (list == NULL) return false; // no memory in system
    if (list[0] == NULL) return false; // no fonts were found

    // load the first font from the list

    ttf_load_from_file(list[0]->filename, &font, false);
    ttf_free_list(list);
    */
    ttf_load_from_file("./simfang.ttf", &font, false);
 
    if (font == NULL) return false;
    return true;
}

//不要忘记在使用完wchar_t*后delete[]释放内存
wchar_t *multi_Byte_To_Wide_Char(const char* pKey)
{
    //string 转 char*
    const char* pCStrKey = pKey;
    //第一次调用返回转换后的字符串长度，用于确认为wchar_t*开辟多大的内存空间
    // int pSize = MultiByteToWideChar(CP_OEMCP, 0, pCStrKey, strlen(pCStrKey) + 1, NULL, 0);
    const int pSize = 1;
    wchar_t *pWCStrKey = (wchar_t *)malloc(pSize*sizeof(wchar_t));
    //第二次调用将单字节字符串转换成双字节字符串
    MultiByteToWideChar(CP_UTF8, 0, pCStrKey, strlen(pCStrKey) + 1, pWCStrKey, pSize);
    return &pWCStrKey[0];
}

static void choose_glyph(const char* pKey)
{
    // find a glyph in the font file
    wchar_t symbol = *multi_Byte_To_Wide_Char(pKey);
    int index = ttf_find_glyph(font, symbol);
    if (index < 0) return ;

    // make mesh object from the glyph
    ttf_glyph_t *glyph = NULL;
    // ttf_mesh_t *mesh = NULL;
    ttf_mesh_t *out;
    if (ttf_glyph2mesh(&font->glyphs[index], &out, TTF_QUALITY_NORMAL, TTF_FEATURES_DFLT) != TTF_DONE) {
        return ;
    }

    // if successful, release the previous object and save the state

    // ttf_free_mesh(mesh);
    glyph = &font->glyphs[index];
    mesh_array_index+=1;
    mesh_array[mesh_array_index] = out;
    return ;
}

bool is_draw_one_chinese = false;
void draw(ttf_mesh_t* mesh) {
    const int view_mode = 0;
    if (view_mode == 0) 
    {
        static int num = 0;
        static int count = 0;
        if (num % 5 == 0 ) {
            count++;
        }
        num++;
        if (count > mesh->nfaces) {
            count= mesh->nfaces;
            is_draw_one_chinese = true;
            count = 0;
        }
        glViewport(draw_start_x ,draw_start_y, square_length_x, square_length_y);
        glColor3f(1, 1, 0);//曲线颜色
        glPolygonMode(GL_FRONT_AND_BACK, view_mode == 1 ? GL_LINE : GL_FILL);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, &mesh->vert->x);
        glDrawElements(GL_TRIANGLES, count * 3,  GL_UNSIGNED_INT, &mesh->faces->v1);
        glDisableClientState(GL_VERTEX_ARRAY);
        glFlush();
    }
    if (view_mode == 1)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glColor3f(1, 1, 0);//曲线颜色
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, mesh->vert);
        glDrawArrays(GL_TRIANGLES, 0, mesh->nvert);
        glDrawElements(GL_TRIANGLES, mesh->nfaces * 3,  GL_UNSIGNED_INT, &mesh->faces->v1);
        glDisableClientState(GL_VERTEX_ARRAY);
        printf("vertex len = %d\n", mesh->nvert);
        glFlush();
    }  
    if (view_mode == 2)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glColor3f(1, 1, 0);//曲线颜色
        glLineWidth(1.0);
        glEnableClientState(GL_VERTEX_ARRAY);
        static int num = 0;
        static int count = 0;
        for (int i = 0; i < mesh->outline->ncontours && i < count; i++)
        {
            glVertexPointer(2, GL_FLOAT, sizeof(ttf_point_t), &mesh->outline->cont[i].pt->x);
            glDrawArrays(GL_LINE_LOOP, 0, mesh->outline->cont[i].length);
        }
        if (num % 500 == 0 ) {
            count++;
        }
        num++;
        glDisableClientState(GL_VERTEX_ARRAY);
        glFlush();
        printf("vertex ncount = %d\n", mesh->outline->ncontours);
    }
    if (view_mode == 3)
    {
        for(int i=0; i<mesh->nfaces; i++)
        {
            printf("%d %d %d\n", mesh->faces[i].v1, mesh->faces[i].v2, mesh->faces[i].v3);
        }
        printf("end \n");
    }
}

void draw_over_one_chinese() {
    if (!is_draw_one_chinese) {
        return;
    }
    is_draw_one_chinese = false;
    choose_glyph("小");
    if ((mesh_array_index-1) % 10 == 0) {
        draw_start_y -= square_length_y;
        draw_start_x = square_length_x;
    }
    draw_start_x += square_length_x*0.7;
    ttf_mesh_t* mesh = mesh_array[mesh_array_index];
    
    // for (int i=0; i<mesh->nvert; i++) {
    //     mesh->vert[i].x = mesh->vert[i].x;
    //     mesh->vert[i].y = mesh->vert[i].y;
    //     printf("%d %f %f \n", i, mesh->vert[i].x, mesh->vert[i].y);
    // }
}

int main( void )
{
    glfwInit();

    //设置OpenGL的版本号，可以不设也别瞎设，设错了图形可能显示不出来
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    //其中第一个参数是窗口宽度，第二个参数是窗口高度，第三个参数是窗口标题，第四个参数是显示模式，NULL为窗口化，如果需要显
    //示全屏，则需要指定覆盖的显示器，第五个参数是设置与那个窗口共享资源，默认为NULL，即不共享资源。
    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello, GLFW", NULL, NULL);
    if (!window)
    {
        glfwTerminate();//清除退出
    }

    if (!load_system_font())
    {
        fprintf(stderr, "unable to load system font\n");
        return 1;
    }
    /* 设置当前的窗口上下文 */
    glfwMakeContextCurrent(window);//上下文啥意思还不太理解？？

    //交换间隔表示交换缓冲区之前等待的帧数，通常称为vsync。
    //默认情况下，交换间隔为0，但因为屏幕每秒只更新60-75次，所以大部分的画面不会被显示。
    //而且，缓冲区有可能在屏幕更新的中间交换，出现屏幕撕裂的情况。
    //所以，可以将该间隔设为1，即每帧更新一次。它可以设置为更高的值，但这可能导致输入延迟。
    glfwSwapInterval(0);

    //检查窗口是否需要关闭，如果需要，这个标志会设置为1。
    //但是，此时窗口没有关闭，所以你需要监视这个标志，及时销毁窗口并给予用户反馈。
    is_draw_one_chinese = true;
    while (!glfwWindowShouldClose(window))
    {
        draw_over_one_chinese();

        mesh_vertext_resort(mesh_array[mesh_array_index]);
        // 绘制模型
        draw(mesh_array[mesh_array_index]);
        //交换缓冲区。GLFW在默认情况下使用两个缓冲区。每个窗口有两个渲染缓冲区——前缓冲区和后缓冲区。
        //前缓冲区是正在显示的缓冲区，后缓冲区是即将显示的缓冲区。
        glfwSwapBuffers(window);

        //glfw处理事件有2个函数：glfwPollEvents()和glfwWaitEvents(),前者会立即处理已经到位的事件，后者等待。
        //当你制作游戏或是动画时，尽量使用轮询。如果相反，你需要在产生事件后才渲染，可是通过等待
        //来处理事件，即glfwWaitEvent，比如制作编辑器的时候，使用等待可以节省大量硬件资源。
        glfwPollEvents();
        usleep(1000);
    }
    glfwDestroyWindow(window);
    glfwTerminate();//清除退出
}
