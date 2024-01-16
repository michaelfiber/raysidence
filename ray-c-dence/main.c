#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

char *server;
char *key;
char time_display[4096];
char temp_display[4096];
int temp = 0;
bool is_snowing = false;

bool is_thinking = false;

Rectangle ripple_last;
Rectangle ripple;
float ripple_alpha = 0.0f;
Color ripple_color;


typedef struct
{
    int type;
    char code[16];
    char name[4096];
} Group;

typedef struct
{
    int hue;
    int sat;
    Color preview;
} HueSat;

HueSat hue_sats[] = {
    {7820, 191, WHITE},
    {207, 254, RED},
    {54776, 249, PURPLE},
    {25500, 249, GREEN},
    {46920, 249, BLUE}};

int hue_sat_count = 5;

Group groups[16];
int group_count = 0;
int current_group = -1;
float time_countdown = -1.0f;
float temp_countdown = -1.0f;

// Accepts server, key, group id, brightness as an int from 0 to 255
char cmd_group_on[] = "curl -s -X PUT http://%s.local/api/%s/groups/%s/action --data '{\"on\":true,\"bri\":%d}'";

// accepts server, key, group id, hue as an int from 0 to 65K, sat as an int from 0 to 255
char cmd_group_hue_sat[] = "curl -s -X PUT http://%s.local/api/%s/groups/%s/action --data '{\"on\":true,\"hue\":%d,\"sat\":%d}'";

// accepts server, key, group id
char cmd_group_off[] = "curl -s -X PUT http://%s.local/api/%s/groups/%s/action --data '{\"on\":false}'";

void draw_snow()
{
    static float alpha = 0.0f;
    static bool inactive = true;
    static Vector2 snow[50] = {0};

    if (inactive)
    {
        inactive = false;
        for (int i = 0; i < 50; i++)
        {
            snow[i].x = GetRandomValue(0, GetScreenWidth());
            snow[i].y = GetRandomValue(0, GetScreenHeight());
        }
    }

    if (is_snowing && alpha < 1.0f)
    {
        alpha += GetFrameTime() * 0.1f;
    }

    if (!is_snowing && alpha > 0.0f)
    {
        alpha -= GetFrameTime() * 0.1f;
        if (alpha < 0.0f)
        {
            alpha = 0.0f;
        }
    }

    if (alpha > 0.0f)
    {
        for (int i = 0; i < 50; i++)
        {
            snow[i].y += GetFrameTime() * 20;
            if (snow[i].y > GetScreenHeight())
            {
                snow[i].x = GetRandomValue(0, GetScreenWidth());
                snow[i].y = -3;
            }

            DrawRectangle(snow[i].x, snow[i].y, 3, 3, Fade(WHITE, alpha));
        }
    }
}

void draw_ripple()
{
    if (ripple.width > GetScreenWidth() * 2)
    {
        return;
    }

    DrawRectangleLinesEx(ripple, 2.0f, Fade(ripple_color, ripple_alpha));
    ripple.width += GetFrameTime() * 1000;
    ripple.height += GetFrameTime() * 1000;
    ripple.x -= GetFrameTime() * 500;
    ripple.y -= GetFrameTime() * 500;
    ripple_alpha -= GetFrameTime();
}

void draw_thinking()
{
    static float rot = 0.0f;
    
    if (is_thinking)
    {
        rot += GetFrameTime() * 100;
        Rectangle rec = (Rectangle){
            GetScreenWidth() / 2 - 20,
            GetScreenHeight() - 40,
            40, 
            40
        };

        DrawRectanglePro(rec, (Vector2){ 20, 20 }, rot, BLUE);

    }
}

void activate_ripple(Rectangle rec, Color c)
{
    ripple_color = c;

    ripple.x = rec.x;
    ripple.y = rec.y;
    ripple.width = rec.width;
    ripple.height = rec.height;

    ripple_last.x = rec.x;
    ripple_last.y = rec.y;
    ripple_last.width = rec.width;
    ripple_last.height = rec.height;

    ripple_alpha = 1.0f;
}

void draw_temp(int x, int y)
{
    temp_countdown -= GetFrameTime();
    static int text_size = 0;

    if (temp_countdown < 0.0f)
    {
        temp_countdown = 1.0f;
        FILE *f = fopen("temp.txt", "r");

        if (f == NULL)
        {
            printf("Could not open temp.txt\n");
            return;
        }

        char c;
        int i = 0;
        temp_display[0] = '\0';

        do
        {
            c = fgetc(f);
            if (c == '\n')
            {
                temp_display[i++] = '\0';
                break;
            }
            temp_display[i++] = c;
        } while (c != EOF && c != 255);

        fclose(f);

        text_size = MeasureText(temp_display, GetFontDefault().baseSize * 3);
    }

    Color c = RED;
    temp = atoi(temp_display);

    if (temp < 33)
    {
        c = BLUE;
    }

    if (temp < 0)
    {
        c = WHITE;
    }

    is_snowing = !(strstr(temp_display, "Snow") == NULL) || !(strstr(temp_display, "snow") == NULL);

    DrawText(temp_display, x - text_size, y, GetFontDefault().baseSize * 3, c);
}

void draw_time(int x, int y)
{
    time_countdown -= GetFrameTime();

    if (time_countdown < 0.0f)
    {
        time_countdown = 1.0f;
        time_t raw_time;
        struct tm *timeinfo;
        time(&raw_time);
        timeinfo = localtime(&raw_time);
        int hours = timeinfo->tm_hour;

        if (hours > 12)
        {
            hours -= 12;
        }

        if (hours < 1)
        {
            hours = 12;
        }

        sprintf(time_display, "%d:%02d:%02d", hours, timeinfo->tm_min, timeinfo->tm_sec);
    }

    DrawText(time_display, x, y, GetFontDefault().baseSize * 5, RED);
}

void *_run_cmd(void *cmd)
{
    system(cmd);
    printf("Thread done\n");
    activate_ripple(ripple_last, GREEN);
    is_thinking = false;
    return NULL;
}

void run_cmd(char *cmd)
{
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, _run_cmd, cmd);
    printf("set_level done\n");
}

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

    run_cmd(cmd);
}

void set_color(int hue_sat)
{
    char cmd[4096];
    sprintf(cmd, cmd_group_hue_sat, server, key, groups[current_group].code, hue_sats[hue_sat].hue, hue_sats[hue_sat].sat);

    run_cmd(cmd);
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
    } while (c != EOF && c != 255);
    fclose(f);
}

void update_top_menu()
{
    int i = 0;
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 2; x++)
        {
            Rectangle rec = (Rectangle){
                25 + x * 380,
                25 + y * 90,
                370,
                80};

            if (IsMouseButtonPressed(0) &&
                CheckCollisionPointRec(GetMousePosition(), rec))
            {
                activate_ripple(rec, GREEN);
                current_group = i;
            }

            DrawRectangleLinesEx(rec, 1.0f, RED);
            DrawText(groups[i].name, 35 + x * 380, 40 + y * 90, 50, RED);
            i++;
            if (i == group_count)
                break;
        }
        if (i == group_count)
            break;
    }

    draw_time(25, 415);
    draw_temp(775, 435);
}

void update_group()
{
    DrawText(groups[current_group].name, 10, 10, 80, RED);

    if (groups[current_group].type == 0)
    {
        for (int i = 0; i < 10; i++)
        {
            Rectangle rec = (Rectangle){
                30 + i * 75, 350, 65, 50};
            Color bg = Fade(WHITE, (float)i / 9.0f);
            DrawRectangleRec(rec, bg);
            DrawRectangleLinesEx(rec, 1.0f, RED);
            if (IsMouseButtonPressed(0) &&
                CheckCollisionPointRec(GetMousePosition(), rec))
            {
                is_thinking = true;
                activate_ripple(rec, MAROON);
                set_level(i);
                current_group = -1;
            }
        }

        for (int i = 0; i < hue_sat_count; i++)
        {
            Rectangle rec = (Rectangle){
                30 + i * 75, 200, 65, 50};
            DrawRectangleRec(rec, hue_sats[i].preview);
            DrawRectangleLinesEx(rec, 1.0f, RED);
            if (IsMouseButtonPressed(0) &&
                CheckCollisionPointRec(GetMousePosition(), rec))
            {
                is_thinking = true;
                activate_ripple(rec, MAROON);
                set_color(i);
                current_group = -1;
            }
        }
    }
    else if (groups[current_group].type == 1)
    {
        Rectangle rec = (Rectangle){
            GetScreenWidth() / 8,
            200,
            GetScreenWidth() / 4,
            GetScreenWidth() / 4};

        DrawRectangleLinesEx(rec, 1.0f, WHITE);

        if (CheckCollisionPointRec(GetMousePosition(), rec) && IsMouseButtonPressed(0))
        {
            is_thinking = true;
            activate_ripple(rec, MAROON);
            run_cmd("./kasa-off.sh");
            //system("./kasa-off.sh");
            current_group = -1;
        }

        rec.x = GetScreenWidth() / 8 * 5;
        DrawRectangleRec(rec, WHITE);

        if (CheckCollisionPointRec(GetMousePosition(), rec) && IsMouseButtonPressed(0))
        {
            is_thinking = true;
            activate_ripple(rec, MAROON);
            run_cmd("./kasa-on.sh");
            //system("./kasa-on.sh");
            current_group = -1;
        }
    }
}

int main(void)
{
    server = getenv("HUE_SERVER");
    key = getenv("HUE_KEY");

    read_groups();

    sprintf(groups[group_count].name, "%s", "Playroom");
    groups[group_count].type = 1;
    sprintf(groups[group_count].code, "%s", "-99");
    group_count++;

    InitWindow(800, 480, "Ray-C-dence");

    SetTargetFPS(30);

    ripple.width = GetScreenWidth() * 2;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        draw_snow();

        draw_ripple();

        draw_thinking();

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
