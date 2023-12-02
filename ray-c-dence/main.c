#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <string.h>

char *server;
char *key;

typedef struct
{
    char code[16];
    char name[4096];
} Group;

Group groups[16];
int group_count = 0;
int current_group = -1;

char cmd_group_on[] = "curl -s -X PUT http://%s.local/api/%s/groups/%s/action --data '{\"on\":true,\"bri\":%d}'";
char cmd_group_off[] = "curl -s -X PUT http://%s.local/api/%s/groups/%s/action --data '{\"on\":false}'";

void set_level(int level)
{
    char cmd[4096];
    int bri = 254 * (float)level / 9.0f;
    if (level > 0)
    {
        sprintf(cmd, cmd_group_on, server, key, groups[current_group].code, bri);
    } 
    else
    {
        sprintf(cmd, cmd_group_off, server, key, groups[current_group].code);
    }
    system(cmd);
}

void read_groups()
{
    FILE *f;
    char c;
    char chunk[4096];
    int i = 0;

    int current_code = -1;

    f = fopen("groups.txt", "r");
    if (NULL == f)
    {
        printf("Could not open groups.txt\n");
        return;
    }

    do
    {
        c = fgetc(f);
        if (c == ':')
        {
            chunk[i] = '\0';
            current_code = atoi(chunk);
            chunk[0] = '\0';
            i = 0;
        }
        else if (c == '\n')
        {
            chunk[i++] = '\0';
            printf("Code: %d Chunk: %s\n", current_code, chunk);
            sprintf(groups[group_count].code, "%d", current_code);
            sprintf(groups[group_count].name, "%s", chunk);
            group_count++;
            chunk[0] = '\0';
            i = 0;
        }
        else
        {
            chunk[i++] = c;
        }
    } while (c != EOF);

    fclose(f);
}

void update_top_menu()
{
    int i = 0;
    for (int y = 0; y < 3; y++)
    {
        for (int x = 0; x < 2; x++)
        {
            if (IsMouseButtonPressed(0) &&
                CheckCollisionPointRec(GetMousePosition(), (Rectangle){
                                                               25 + x * 380,
                                                               25 + y * 110,
                                                               370,
                                                               100}))
            {
                current_group = i;
            }

            DrawRectangle(25 + x * 380, 25 + y * 110, 370, 100, MAROON);
            DrawText(groups[i].name, 35 + x * 380, 40 + y * 110, 50, BLACK);
            i++;
            if (i == group_count)
                break;
        }
        if (i == group_count)
            break;
    }
}

void update_group()
{
    DrawText(groups[current_group].name, 10, 10, 80, MAROON);

    for (int i = 0; i < 10; i++)
    {
        Color bg = Fade(WHITE, (float)i / 9.0f);
        DrawRectangle(30 + i * 75, 350, 65, 50, bg);
        DrawRectangleLines(30 + i * 75, 350, 65, 50, MAROON);
        if (IsMouseButtonPressed(0) &&
            CheckCollisionPointRec(GetMousePosition(), (Rectangle){
                                                           30 + i * 75, 350, 65, 50}))
        {
            // set the group brightness
            set_level(i);
            current_group = -1;
        }
    }
}

int main(void)
{
    server = getenv("HUE_SERVER");
    key = getenv("HUE_KEY");

    read_groups();

    InitWindow(800, 480, "Ray-C-dence");

    SetTargetFPS(30);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        if (current_group == -1)
        {
            update_top_menu();
        }
        else
        {
            update_group();
        }
        EndDrawing();
    }

    CloseWindow();
}
