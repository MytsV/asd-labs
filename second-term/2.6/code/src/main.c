#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <math.h>
#include "../headers/matrix_tools.h"
#include <string.h>
#include <limits.h>

//булева змінна, що задається користувачем
int directed = 0;

const char APP_NAME[] = "Graph";

/*
КОНСТАНТИ РОЗМІРІВ ГРАФУ
*/

const int MARGIN = 115; //відступи по краях
const int NODE_RADIUS = 35;
const int NODE_SPACING = NODE_RADIUS * 3; //відступи між вершинами

const int MARGIN_TREE = 20;
const int NODE_RADIUS_TREE = 25;
const int NODE_SPACING_TREE = NODE_RADIUS_TREE * 3;

/*
ПАРАМЕТРИ МАЛЮВАННЯ ЗВ'ЯЗКІВ
*/

const int LINE_WIDTH = 2; //товщина обведення
const int CURVE_HOISTING = 20; //підняття кривої лінії над центром вершини
const int SELF_CONNECT_HOISTING = 20; //підняття лінії при з'єднанні вершини з самою собою

const int DOUBLE_OFFSET = 7; //зміщення ліній при з'єднанні двох вершин двічі

const int OFFSET_MULTIPLIER_TILT = 8; //множник зміщення ліній
const int OFFSET_MULTIPLIER_CURVE = 2;

const int ARROW_LENGTH = 18;
const double ARROW_ANGLE = M_PI / 6;

const double WINDOW_HEIGHT_OFFSET_SIZE = 0.6; //частина вікна (у відношенні), в якій деякі зв'язки будуть зі зміщенням вверх

const double WEIGHT_PADDING = 5;
const double WEIGHT_FONT_SIZE = 13;

/*
ПАРАМЕТРИ ВІДОБРАЖННЯ НИЖНЬОЇ КНОПКИ
*/

const int BUTTON_HEIGHT = 100;
const int BUTTON_MARGIN = 20;
const int BUTTON_ROW = 7;

typedef struct color //тут значення RGB є нормованими, тобто діляться на 0xFF
{
    double r;
    double g;
    double b;
}
color_t;

//індекс кольору в масиві відповідний значенню константи вище
color_t colors[] = {{1, 1, 1}, {0.925, 0.678, 0.654}};
color_t colors_border[] = {{0, 0, 0}, {0.850, 0.282, 0.227}};

//координати вершини
typedef struct node_pos
{
    double x;
    double y;
} node_pos_t;

//кількість вершин графа на конжній стороні поля
typedef struct field
{
    int left;
    int right;
    int top;
    int bottom;
} field_t;

int window_width;
int window_height;

struct field window_field;

double **matrix;
double **weights;

node_pos_t *positions;

int *in_set;
double **tree_matrix;
int edges_visited = 0;
int last_index = 0;

int search_end = 0;
int show_tree = 0; //параметр, що зазначає, чи показується дерево

//ми окремо задаватимемо ці параметри, не використовуючи константи в коді, оскільки для відображення дерева їх потрібно буде змінювати
int node_radius;
int node_spacing;
int margin;

void draw_weight(cairo_t *cr, double weight, double x, double y)
{
    cairo_move_to(cr, x + WEIGHT_PADDING, y);

    char text[5];
    sprintf(text, "%d", (int)weight);

    cairo_set_font_size(cr, WEIGHT_FONT_SIZE);
    cairo_show_text(cr, text);
}

void draw_arrow(cairo_t *cr, double start_x, double start_y, double end_x, double end_y)
{
    if (!directed)
    {
        return;
    }
    cairo_stroke(cr); //промальовуємо з'єднання

    double dx = start_x - end_x;
    double dy = start_y - end_y;
    double length = sqrt(dx * dx + dy * dy); //довжина лінії
    double ratio = ARROW_LENGTH / length;    //відношення довжини боку стрілки до довжини лінії

    cairo_new_path(cr);                                                               //створюємо новий шлях малювання задля того, щоб не замальовувалися інші елементи графу
    double x_first = end_x + ratio * (dx * cos(ARROW_ANGLE) + dy * sin(ARROW_ANGLE)); //розраховуємо за формулою координати кінців стрілки
    double y_first = end_y + ratio * (dy * cos(ARROW_ANGLE) - dx * sin(ARROW_ANGLE));
    cairo_move_to(cr, x_first, y_first);
    cairo_line_to(cr, x_first, y_first);
    cairo_line_to(cr, end_x, end_y);

    double x_second = end_x + ratio * (dx * cos(ARROW_ANGLE) - dy * sin(ARROW_ANGLE));
    double y_second = end_y + ratio * (dy * cos(ARROW_ANGLE) + dx * sin(ARROW_ANGLE));
    cairo_line_to(cr, x_second, y_second);
    cairo_line_to(cr, x_first, y_first);

    cairo_close_path(cr);
    cairo_stroke_preserve(cr);
    cairo_fill(cr);
}

void connect_with_self(cairo_t *cr, int i)
{
    node_pos_t node_n = positions[i];
    int y_offset_sign = node_n.y < window_height * WINDOW_HEIGHT_OFFSET_SIZE ? -1 : 1;

    double start_x = node_n.x;
    double start_y = node_n.y + y_offset_sign * node_radius;

    double end_x = node_n.x + node_radius;
    double end_y = node_n.y;

    double middle_x = end_x;
    double middle_y = end_y + y_offset_sign * SELF_CONNECT_HOISTING;

    cairo_move_to(cr, start_x, start_y);
    cairo_line_to(cr, start_x, start_y + y_offset_sign * SELF_CONNECT_HOISTING);
    cairo_line_to(cr, middle_x, middle_y);
    cairo_line_to(cr, end_x, end_y);

    draw_arrow(cr, middle_x, middle_y, end_x, end_y);
    draw_weight(cr, weights[i][i], middle_x, middle_y);
}

void connect_horizontal(cairo_t *cr, int i, int j, double offset)
{
    node_pos_t node_n = positions[i];
    node_pos_t node_m = positions[j];
    double dx = node_m.x - node_n.x;

    //якщо вершини не є сусідніми, ми проведемо між ними криву
    if (fabs(dx) > node_spacing * 2)
    {
        int y_offset_sign = node_n.y < window_height * WINDOW_HEIGHT_OFFSET_SIZE ? -1 : 1;

        double y_margin = y_offset_sign * CURVE_HOISTING;                        //піднімаємо криву від центра кола, зміщення завжди буде вгору
        double x_margin = sqrt(node_radius * node_radius - y_margin * y_margin); //розраховуємо зміщення по x за допомогою теореми Піфагора
        x_margin = dx >= 0 ? x_margin : -x_margin;                               //домножуємо зміщення лише по x на знак різниці координат

        double start_x = node_n.x + x_margin;
        double start_y = node_n.y + y_margin;

        double middle_x = node_n.x + dx / 2; //середина кривої буде розміщуватися рівно між вершинами по x
        //і буде піднята на висоту стандартної відстані між вершинами
        double middle_y = node_n.y + y_offset_sign * node_spacing + y_margin + OFFSET_MULTIPLIER_CURVE * offset * DOUBLE_OFFSET;

        double end_x = node_m.x - x_margin;
        double end_y = node_m.y + y_margin;

        cairo_move_to(cr, start_x, start_y);
        cairo_curve_to(cr, start_x, start_y, middle_x, middle_y, end_x, end_y);

        draw_arrow(cr, middle_x, middle_y, end_x, end_y);
        draw_weight(cr, weights[i][j], middle_x, start_y + y_margin * 1.5);
    }
    else
    {
        double y_margin = offset * DOUBLE_OFFSET;
        double x_margin = sqrt(node_radius * node_radius - y_margin * y_margin);
        x_margin = dx >= 0 ? x_margin : -x_margin;

        double start_x = node_n.x + x_margin;
        double start_y = node_n.y + y_margin;

        double end_x = node_m.x - x_margin;
        double end_y = node_m.y + y_margin;

        cairo_move_to(cr, start_x, start_y);
        cairo_line_to(cr, end_x, end_y);

        draw_arrow(cr, start_x, start_y, end_x, end_y);
        draw_weight(cr, weights[i][j], node_n.x + dx / 2, start_y - WEIGHT_PADDING);
    }
}

void connect_vertical(cairo_t *cr, int i, int j, double offset)
{
    node_pos_t node_n = positions[i];
    node_pos_t node_m = positions[j];
    double dy = node_m.y - node_n.y;

    //якщо вершини не є сусідніми, ми проведемо між ними криву
    if (fabs(dy) > node_spacing * 2)
    {
        double x_margin = -CURVE_HOISTING; //зсуваємо криву від центра кола вліво
        //розраховуємо зміщення по y за допомогою теореми Піфагора
        double y_margin = sqrt(node_radius * node_radius - x_margin * x_margin);
        y_margin = dy >= 0 ? y_margin : -y_margin; //домножуємо зміщення лише по x на знак різниці координат

        double start_x = node_n.x + x_margin;
        double start_y = node_n.y + y_margin;

        //і буде зміщена вліво на стандартну відстань між вершинами
        double middle_x = node_n.x - node_spacing * 2 - x_margin + OFFSET_MULTIPLIER_CURVE * offset * DOUBLE_OFFSET;
        double middle_y = node_n.y + dy / 2; //середина кривої буде розміщуватися рівно між вершинами по y

        double end_x = node_m.x + x_margin;
        double end_y = node_m.y - y_margin;

        cairo_move_to(cr, start_x, start_y);
        cairo_curve_to(cr, start_x, start_y, middle_x, middle_y, end_x, end_y);

        draw_arrow(cr, middle_x, middle_y, end_x, end_y);
        draw_weight(cr, weights[i][j], middle_x + WEIGHT_PADDING, middle_y);
    }
    else
    {
        double x_margin = offset * DOUBLE_OFFSET;
        double y_margin = sqrt(node_radius * node_radius - x_margin * x_margin);
        y_margin = dy >= 0 ? y_margin : -y_margin;

        double start_x = node_n.x + x_margin;
        double start_y = node_n.y + y_margin;

        double end_x = node_m.x + x_margin;
        double end_y = node_m.y - y_margin;

        cairo_move_to(cr, start_x, start_y);
        cairo_line_to(cr, end_x, end_y);

        draw_arrow(cr, start_x, start_y, end_x, end_y);
        draw_weight(cr, weights[i][j], start_x + WEIGHT_PADDING, node_n.y + dy / 2);
    }
}

void connect_tilted(cairo_t *cr, int i, int j, double offset)
{
    node_pos_t node_n = positions[i];
    node_pos_t node_m = positions[j];

    //різниця між координатами зі знаком
    double dx = node_m.x - node_n.x;
    double dy = node_m.y - node_n.y;

    //нахил до осі y
    double tangent = (double)fabs(dx) / fabs(dy);

    //розрахунок зміщення від центру кола до його краю за теоремою Піфагора
    double y_margin = sqrt((node_radius * node_radius) / (1 + tangent * tangent));
    double x_margin = y_margin * tangent;

    //перераховуємо знак
    y_margin = dy >= 0 ? y_margin : -y_margin;
    x_margin = dx >= 0 ? x_margin : -x_margin;

    double start_x = node_n.x + x_margin;
    double start_y = node_n.y + y_margin;

    double middle_x = node_n.x + dx / 2 + OFFSET_MULTIPLIER_TILT * offset * DOUBLE_OFFSET;
    double middle_y = node_n.y + dy / 2;

    //зберігаємо координати кінця задля перевикористання їх при малюванні стрілки
    double end_x = node_m.x - x_margin;
    double end_y = node_m.y - y_margin;

    cairo_move_to(cr, start_x, start_y);
    cairo_line_to(cr, middle_x, middle_y);
    cairo_line_to(cr, end_x, end_y);

    draw_arrow(cr, middle_x, middle_y, end_x, end_y);

    double weight_x = offset != 0 ? middle_x : start_x;
    double weight_y = offset != 0 ? middle_y : start_y;

    draw_weight(cr, weights[i][j], weight_x + (dx >= 0 ? WEIGHT_PADDING : -WEIGHT_PADDING), weight_y + (dy < 0 ? WEIGHT_PADDING : -WEIGHT_PADDING));
}

void connect_nodes(cairo_t *cr, int i, int j, double offset)
{
    node_pos_t node_n = positions[i];
    node_pos_t node_m = positions[j];

    if (node_n.x == node_m.x)
    {
        if (node_n.y == node_m.y)
        {
            connect_with_self(cr, i);
        }
        else
        {
            connect_vertical(cr, i, j, offset);
        }
    }
    else if (node_n.y == node_m.y)
    {
        connect_horizontal(cr, i, j, offset);
    }
    else
    {
        connect_tilted(cr, i, j, offset);
    }
    cairo_stroke(cr);
}

void set_side_positions(int node_count, int *index, node_pos_t (*get_pos)(int))
{
    int spaced = 0;
    for (int i = 0; i < node_count; i++)
    {
        node_pos_t pos = get_pos(spaced);
        positions[*index] = pos;
        *index += 1;
        spaced++;
    }
}

node_pos_t get_top_position(int spaced)
{
    node_pos_t pos;
    pos.x = margin + (node_radius * 2 + node_spacing) * spaced + node_radius;
    pos.y = margin + node_radius;
    return pos;
}

node_pos_t get_right_position(int spaced)
{
    node_pos_t pos;
    pos.x = window_width - margin - node_radius;
    pos.y = margin + (node_radius * 2 + node_spacing) * (spaced + 1) + node_radius;
    return pos;
}

node_pos_t get_bottom_position(int spaced)
{
    node_pos_t pos;
    pos.x = window_width - (margin + (node_radius * 2 + node_spacing) * (spaced + 1)) - node_radius;
    pos.y = window_height - margin - node_radius;
    return pos;
}

node_pos_t get_left_position(int spaced)
{
    node_pos_t pos;
    pos.x = margin + node_radius;
    pos.y = window_height - (margin + (node_radius * 2 + node_spacing) * (spaced + 1)) - node_radius;
    return pos;
}

node_pos_t get_center_position(int spaced)
{
    node_pos_t pos;
    pos.x = window_width / 2;
    pos.y = window_height / 2;
    return pos;
}

void get_node_positions()
{
    positions = malloc(sizeof(node_pos_t) * NODE_COUNT);

    int index = 0;
    set_side_positions(window_field.top, &index, get_top_position);

    set_side_positions(window_field.right - 1, &index, get_right_position);

    set_side_positions(window_field.bottom - 1, &index, get_bottom_position);

    set_side_positions(window_field.left - 2, &index, get_left_position);

    set_side_positions(1, &index, get_center_position);
}

void get_node_positions_tree() 
{
    positions = malloc(sizeof(node_pos_t) * NODE_COUNT);

    int level = 0; //ми визначаємо позицію рівнями
    int level_node_count = 1;
    int level_nodes[NODE_COUNT]; 
    int used[NODE_COUNT];

    level_nodes[0] = 0;

    while(level_node_count != 0)
    {
        int x_spaced = 0;
        for (int i = 0; i < level_node_count; i++)
        {
            node_pos_t pos;
            double edge_x = node_radius * (level_node_count - 1) + node_spacing * (level_node_count - 1) / 2;

            pos.x = window_width/2 + x_spaced * (node_spacing + node_radius * 2) - edge_x; 
            pos.y = margin + level * node_spacing + node_radius;
            positions[level_nodes[i]] = pos;

            x_spaced++;
        }

        int new_node_count = 0;
        int new_level_nodes[NODE_COUNT];

        for (int i = 0; i < level_node_count; i++)
        {
            for (int j = 0; j < NODE_COUNT; j++)
            {
                if (matrix[level_nodes[i]][j])
                {
                    new_level_nodes[new_node_count] = j;
                    new_node_count++;
                }
            }
        }

        for (int i = 0; i < new_node_count; i++) 
        {
            level_nodes[i] = new_level_nodes[i];
        }

        level_node_count = new_node_count;
        level++;
    }
}

void set_node_border(cairo_t *cr, int status) 
{
    color_t color = colors_border[status];
    cairo_set_source_rgb(cr, color.r, color.g, color.b);
}

void draw_connections(cairo_t *cr, double **matrix)
{
    for (int i = 0; i < NODE_COUNT; i++)
    {
        for (int j = 0; j < NODE_COUNT; j++)
        {
            if (!matrix[i][j])
                continue; //якщо в матриці на позиції стоїть нуль, пропускаємо з'єднання

            set_node_border(cr, tree_matrix[i][j] || tree_matrix[j][i]);

            if (directed && i != j && matrix[j][i] == 1)
            {
                //перевіряємо, чи є подвійне з'єднання вершин
                if (i < j)
                {
                    //виключаємо повторне малювання зв'язків
                    connect_nodes(cr, i, j, 1);
                }
            }
            else if (directed || i <= j)
            {
                connect_nodes(cr, i, j, j == (NODE_COUNT - 1) && !show_tree ? 1 : 0); //якщо ми з'єднуємо вершину з серединною, то малюємо зігнуту лінію
            }
        }
    }
}

void set_node_color(cairo_t *cr, int status) 
{
    color_t color = colors[status];
    cairo_set_source_rgb(cr, color.r, color.g, color.b);
    cairo_fill(cr);
}

void draw_node(cairo_t *cr, node_pos_t pos, char *text, int node_in_set)
{
    cairo_move_to(cr, pos.x + node_radius, pos.y);
    set_node_border(cr, node_in_set);
    cairo_arc(cr, pos.x, pos.y, node_radius, 0, 2 * M_PI); //малюємо еліпс

    cairo_stroke_preserve(cr);
    set_node_color(cr, node_in_set);

    set_node_border(cr, node_in_set);
    cairo_set_font_size(cr, node_radius);

    if (strlen(text) > 1) //зважаємо на правильне розміщення вершин з номером більшим за 9
    {
        cairo_move_to(cr, pos.x - node_radius / 2, pos.y + node_radius / 3);
    }
    else
    {
        cairo_move_to(cr, pos.x - node_radius / 3.5, pos.y + node_radius / 3);
    }
    cairo_show_text(cr, text);
}

void draw_nodes(cairo_t *cr)
{
    for (int i = 1; i <= NODE_COUNT; i++)
    {
        char text[3];
        sprintf(text, "%d", i);

        draw_node(cr, positions[i - 1], text, in_set[i - 1]);
    }
}

void draw_graph(cairo_t *cr, double **matrix)
{
    if (show_tree) {
        get_node_positions_tree();
    } else {
        get_node_positions();
    }
    
    cairo_set_source_rgb(cr, 0.294, 0.145, 0.596); //встановлюємо забарвлення

    draw_nodes(cr);
    draw_connections(cr, matrix);

    free(positions);
}

void set_window_size()
{
    //враховуємо відступи і радіус вершин
    window_height = 2 * margin + window_field.right * node_radius * 2 + node_spacing * (window_field.right - 1);
    window_width = 2 * margin + window_field.bottom * node_radius * 2 + node_spacing * (window_field.bottom - 1);
}

void calculate_size()
{
    int free_count = NODE_COUNT - 4 - 1; //віднімаємо кількість крайніх вершин і вершину в центрі
    int vertical = 2 + free_count / 4;

    //мінімальну кількість вершин ставимо усім сторонам
    window_field.left = vertical;
    window_field.right = vertical;
    window_field.top = vertical;
    window_field.bottom = vertical;

    //розподіляємо залишкові вершини на горизонтальні осі
    int lefover = free_count % 4;
    window_field.top += lefover / 2;
    window_field.bottom += lefover - lefover / 2;

    //призначаємо вікну розмір на основі розрахунків
    set_window_size();
}

static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    draw_graph(cr, matrix);
    return FALSE;
}

void get_spanning_tree() //алгоритм Пріма
{
    for (int i = last_index; i < NODE_COUNT; i++)
    {
        if (!in_set[i])
            continue;

        int distance = INT_MAX;
        int x = -1;
        int y = -1;

        for (int j = 0; j < NODE_COUNT; j++)
        {
            if (matrix[i][j] && !in_set[j] && weights[i][j] < distance)
            {
                x = i;
                y = j;
                distance = weights[i][j];
            }
        }

        if (x == -1 || y == -1)
            continue;

        tree_matrix[x][y] = 1;
        in_set[y] = 1;
        edges_visited++;
        if (i < NODE_COUNT - 1)
        {
            last_index = i + 1;
        }
        else
        {
            last_index = 0;
        }
        return;
    }

    last_index = 0;
}

void init_tree(GtkWidget *widget) {
    matrix = tree_matrix;
        
    gtk_widget_hide(widget);

    margin = MARGIN_TREE;
    node_radius = NODE_RADIUS_TREE;
    node_spacing = NODE_SPACING_TREE;

    output_matrix(NODE_COUNT, NODE_COUNT, matrix);
    directed = 1;
    show_tree = 1;
}

void on_search_end(GtkWidget *widget) {
    gtk_button_set_label(GTK_BUTTON(widget), "Показати дерево обходу");
    gtk_widget_queue_draw(widget);
    search_end = 1;
}

static void on_clicked(GtkWidget *widget, gpointer data)
{
    if (edges_visited >= NODE_COUNT - 1)
    {
        if (search_end)
        {
            init_tree(widget);
        }
        else
        {
            on_search_end(widget);
        }
    }
    else
    {
        get_spanning_tree();
    }
}

GtkWidget *create_window(GtkApplication *app)
{
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), APP_NAME);                                                               //встановлюємо ім'я програми
    gtk_window_set_default_size(GTK_WINDOW(window), window_width + BUTTON_MARGIN * 2, window_height + BUTTON_HEIGHT); //встановлюємо розмір вікна

    return window;
}

GtkWidget *create_darea(GtkWidget *window)
{
    GtkWidget *darea = gtk_drawing_area_new();

    //призначаємо колбек для рисування об'єктів
    g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), NULL);
    //призначаємо колбек, який викликається при виході з програми
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    return darea;
}

GtkWidget *create_grid(GtkWidget *darea, GtkWidget *button_next)
{
    GtkWidget *grid = gtk_grid_new();
    gtk_widget_set_margin_start(grid, BUTTON_MARGIN);
    gtk_widget_set_margin_end(grid, BUTTON_MARGIN);
    gtk_widget_set_margin_bottom(grid, BUTTON_MARGIN);

    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);

    gtk_grid_attach(GTK_GRID(grid), button_next, 0, BUTTON_ROW, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), darea, 0, 0, 1, BUTTON_ROW);

    return grid;
}

void on_app_activate(GtkApplication *app, gpointer data)
{
    GtkWidget *window = create_window(app);
    //створюється площина для малювання, яка прив'язується до вікна
    GtkWidget *darea = create_darea(window);

    GtkWidget *button_next = gtk_button_new_with_label("Далі");
    g_signal_connect(G_OBJECT(button_next), "clicked", G_CALLBACK(on_clicked), NULL);

    gtk_container_add(GTK_CONTAINER(window), create_grid(darea, button_next));

    gtk_widget_show_all(window);

    gtk_main();
}

void create_application(int argc, char *argv[])
{
    GtkApplication *app = gtk_application_new("victoria.myts.graphs", G_APPLICATION_FLAGS_NONE);
    //призначаємо колбек, що викликатиметься при запуску GUI програми
    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);

    //запускаємо GUI програму
    g_application_run(G_APPLICATION(app), argc, argv);
}

void init_spanning_values()
{
    in_set = (int *)malloc(sizeof(int) * NODE_COUNT);
    tree_matrix = (double **)malloc(sizeof(double *) * NODE_COUNT);

    for (int i = 0; i < NODE_COUNT; i++)
    {
        in_set[i] = 0;
        double *row = (double *)malloc(sizeof(double) * NODE_COUNT);
        tree_matrix[i] = row;
        for (int j = 0; j < NODE_COUNT; j++)
        {
            tree_matrix[i][j] = 0;
        }
    }

    in_set[0] = 1; //перша вершина завжди включається в кістяк
}

void init_graph_values()
{
    node_radius = NODE_RADIUS;
    node_spacing = NODE_SPACING;
    margin = MARGIN;
}

int main(int argc, char *argv[])
{
    init_graph_values();

    calculate_size();

    matrix = get_matrix();
    weights = get_weight_matrix(matrix, NODE_COUNT, NODE_COUNT);
    to_undirected(matrix);
    output_matrix(NODE_COUNT, NODE_COUNT, matrix);

    printf("\nWeights:\n");

    output_matrix(NODE_COUNT, NODE_COUNT, weights);

    init_spanning_values();
    create_application(argc, argv);

    free_matrix(NODE_COUNT, matrix);

    return 1;
}