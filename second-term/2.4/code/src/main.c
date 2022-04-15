#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <math.h>
#include "../headers/matrix_tools.h"
#include "../headers/graph_operations.h"

//булева змінна, що задається користувачем
int directed;
//індекс вершини, для якої будуть відображені з'єднання (-1 - для всіх)
int node_shown;

const char APP_NAME_DIRECTED[] = "Directed Graph";
const char APP_NAME_UNDIRECTED[] = "Undirected Graph";

const int MARGIN = 50;	//відступи по краях
const int NODE_RADIUS = 35;
const int NODE_SPACING = NODE_RADIUS * 3;	//відступи між вершинами

const int LINE_WIDTH = 2;	//товщина обведення
const int CURVE_HOISTING = 20;	//підняття кривої лінії над центром вершини
const int SELF_CONNECT_HOISTING = 20;	//підняття лінії при з'єднанні вершини з самою собою

const int DOUBLE_OFFSET = 7;	//зміщення ліній при з'єднанні двох вершин двічі

const int OFFSET_MULTIPLIER_TILT = 8; //множник зміщення ліній 
const int OFFSET_MULTIPLIER_CURVE = 2;

const int ARROW_LENGTH = 18;
const double ARROW_ANGLE = M_PI / 6;

const double WINDOW_HEIGHT_OFFSET_SIZE = 0.6; //частина вікна (у відношенні), в якій деякі зв'язки будуть зі зміщенням вверх

int node_count;

//координати вершини
typedef struct node_pos
{
    double x;
    double y;
}
node_pos_t;

//кількість вершин графа на конжній стороні поля
typedef struct field
{
    int left;
    int right;
    int top;
    int bottom;
}
field_t;

int window_width;
int window_height;

struct field window_field;

double **matrix;

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
    double ratio = ARROW_LENGTH / length; //відношення довжини боку стрілки до довжини лінії

    cairo_new_path(cr); //створюємо новий шлях малювання задля того, щоб не замальовувалися інші елементи графу
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

void connect_with_self(cairo_t *cr, node_pos_t node_n)
{
    int y_offset_sign = node_n.y < window_height * WINDOW_HEIGHT_OFFSET_SIZE ? -1 : 1;

    double start_x = node_n.x;
    double start_y = node_n.y + y_offset_sign * NODE_RADIUS;

    double end_x = node_n.x + NODE_RADIUS;
    double end_y = node_n.y;

    double middle_x = end_x;
    double middle_y = end_y + y_offset_sign * SELF_CONNECT_HOISTING;

    cairo_move_to(cr, start_x, start_y);
    cairo_line_to(cr, start_x, start_y + y_offset_sign * SELF_CONNECT_HOISTING);
    cairo_line_to(cr, middle_x, middle_y);
    cairo_line_to(cr, end_x, end_y);

    draw_arrow(cr, middle_x, middle_y, end_x, end_y);
}

void connect_horizontal(cairo_t *cr, node_pos_t node_n, node_pos_t node_m, double offset)
{
    double dx = node_m.x - node_n.x;

   	//якщо вершини не є сусідніми, ми проведемо між ними криву
    if (fabs(dx) > NODE_SPACING * 2)
    {
        int y_offset_sign = node_n.y < window_height * WINDOW_HEIGHT_OFFSET_SIZE ? -1 : 1;

        double y_margin = y_offset_sign * CURVE_HOISTING; //піднімаємо криву від центра кола, зміщення завжди буде вгору
        double x_margin = sqrt(NODE_RADIUS * NODE_RADIUS - y_margin * y_margin); //розраховуємо зміщення по x за допомогою теореми Піфагора
        x_margin = dx >= 0 ? x_margin : -x_margin;	//домножуємо зміщення лише по x на знак різниці координат

        double start_x = node_n.x + x_margin;
        double start_y = node_n.y + y_margin;

        double middle_x = node_n.x + dx / 2; //середина кривої буде розміщуватися рівно між вершинами по x
        //і буде піднята на висоту стандартної відстані між вершинами
        double middle_y = node_n.y + y_offset_sign * NODE_SPACING + y_margin + OFFSET_MULTIPLIER_CURVE * offset * DOUBLE_OFFSET;

        double end_x = node_m.x - x_margin;
        double end_y = node_m.y + y_margin;

        cairo_move_to(cr, start_x, start_y);
        cairo_curve_to(cr, start_x, start_y, middle_x, middle_y, end_x, end_y);

        draw_arrow(cr, middle_x, middle_y, end_x, end_y);
    }
    else
    {
        double y_margin = offset * DOUBLE_OFFSET;
        double x_margin = sqrt(NODE_RADIUS * NODE_RADIUS - y_margin * y_margin);
        x_margin = dx >= 0 ? x_margin : -x_margin;

        double start_x = node_n.x + x_margin;
        double start_y = node_n.y + y_margin;

        double end_x = node_m.x - x_margin;
        double end_y = node_m.y + y_margin;

        cairo_move_to(cr, start_x, start_y);
        cairo_line_to(cr, end_x, end_y);

        draw_arrow(cr, start_x, start_y, end_x, end_y);
    }
}

void connect_vertical(cairo_t *cr, node_pos_t node_n, node_pos_t node_m, double offset)
{
    double dy = node_m.y - node_n.y;

   	//якщо вершини не є сусідніми, ми проведемо між ними криву
    if (fabs(dy) > NODE_SPACING * 2)
    {
        double x_margin = -CURVE_HOISTING; //зсуваємо криву від центра кола вліво
        //розраховуємо зміщення по y за допомогою теореми Піфагора
        double y_margin = sqrt(NODE_RADIUS * NODE_RADIUS - x_margin * x_margin);	
        y_margin = dy >= 0 ? y_margin : -y_margin; //домножуємо зміщення лише по x на знак різниці координат

        double start_x = node_n.x + x_margin;
        double start_y = node_n.y + y_margin;

        //і буде зміщена вліво на стандартну відстань між вершинами
        double middle_x = node_n.x - NODE_SPACING * 2 - x_margin + OFFSET_MULTIPLIER_CURVE * offset * DOUBLE_OFFSET;
        double middle_y = node_n.y + dy / 2; //середина кривої буде розміщуватися рівно між вершинами по y

        double end_x = node_m.x + x_margin;
        double end_y = node_m.y - y_margin;

        cairo_move_to(cr, start_x, start_y);
        cairo_curve_to(cr, start_x, start_y, middle_x, middle_y, end_x, end_y);

        draw_arrow(cr, middle_x, middle_y, end_x, end_y);
    }
    else
    {
        double x_margin = offset * DOUBLE_OFFSET;
        double y_margin = sqrt(NODE_RADIUS * NODE_RADIUS - x_margin * x_margin);
        y_margin = dy >= 0 ? y_margin : -y_margin;

        double start_x = node_n.x + x_margin;
        double start_y = node_n.y + y_margin;

        double end_x = node_m.x + x_margin;
        double end_y = node_m.y - y_margin;

        cairo_move_to(cr, start_x, start_y);
        cairo_line_to(cr, end_x, end_y);

        draw_arrow(cr, start_x, start_y, end_x, end_y);
    }
}

void connect_tilted(cairo_t *cr, node_pos_t node_n, node_pos_t node_m, double offset)
{
   	//різниця між координатами зі знаком
    double dx = node_m.x - node_n.x;
    double dy = node_m.y - node_n.y;

   	//нахил до осі y
    double tangent = (double) fabs(dx) / fabs(dy);

   	//розрахунок зміщення від центру кола до його краю за теоремою Піфагора
    double y_margin = sqrt((NODE_RADIUS * NODE_RADIUS) / (1 + tangent * tangent));
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
}

void connect_nodes(cairo_t *cr, node_pos_t node_n, node_pos_t node_m, double offset)
{
    if (node_n.x == node_m.x)
    {
        if (node_n.y == node_m.y)
        {
            connect_with_self(cr, node_n);
        }
        else
        {
            connect_vertical(cr, node_n, node_m, offset);
        }
    }
    else if (node_n.y == node_m.y)
    {
        connect_horizontal(cr, node_n, node_m, offset);
    }
    else
    {
        connect_tilted(cr, node_n, node_m, offset);
    }
    cairo_stroke(cr);
}

void set_side_positions(node_pos_t *positions, int node_count, int *index, node_pos_t(*get_pos)(int))
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
    pos.x = MARGIN + (NODE_RADIUS * 2 + NODE_SPACING) * spaced + NODE_RADIUS;
    pos.y = MARGIN + NODE_RADIUS;
    return pos;
}

node_pos_t get_right_position(int spaced)
{
    node_pos_t pos;
    pos.x = window_width - MARGIN - NODE_RADIUS;
    pos.y = MARGIN + (NODE_RADIUS * 2 + NODE_SPACING) * (spaced + 1) + NODE_RADIUS;
    return pos;
}

node_pos_t get_bottom_position(int spaced)
{
    node_pos_t pos;
    pos.x = window_width - (MARGIN + (NODE_RADIUS * 2 + NODE_SPACING) * (spaced + 1)) - NODE_RADIUS;
    pos.y = window_height - MARGIN - NODE_RADIUS;
    return pos;
}

node_pos_t get_left_position(int spaced)
{
    node_pos_t pos;
    pos.x = MARGIN + NODE_RADIUS;
    pos.y = window_height - (MARGIN + (NODE_RADIUS * 2 + NODE_SPACING) * (spaced + 1)) - NODE_RADIUS;
    return pos;
}

node_pos_t get_center_position(int spaced)
{
    node_pos_t pos;
    pos.x = window_width / 2;
    pos.y = window_height / 2;
    return pos;
}

node_pos_t *get_node_positions()
{
    node_pos_t *positions = (node_pos_t *)malloc(sizeof(node_pos_t) * node_count);

    int index = 0;
    set_side_positions(positions, window_field.top, &index, get_top_position);

    set_side_positions(positions, window_field.right - 1, &index, get_right_position);

    set_side_positions(positions, window_field.bottom - 1, &index, get_bottom_position);

    set_side_positions(positions, window_field.left - 2, &index, get_left_position);

    set_side_positions(positions, 1, &index, get_center_position);

    return positions;
}

void draw_connections(cairo_t *cr, node_pos_t *positions, double **matrix)
{
   	//малюємо з'єднання лише для вершини (вершин), заданої користувачем
    int start_index = node_shown == -1 ? 0 : node_shown;
    int end_index = node_shown == -1 ? node_count : node_shown + 1;

    for (int i = start_index; i < end_index; i++)
    {
        for (int j = 0; j < node_count; j++)
        {
            if (!matrix[i][j]) continue; //якщо в матриці на позиції стоїть нуль, пропускаємо з'єднання

            if (directed && i != j && matrix[j][i] == 1)
            {
            	//перевіряємо, чи є подвійне з'єднання вершин
                if (i < j || j < start_index)
                {
                	//виключаємо повторне малювання зв'язків
                    connect_nodes(cr, positions[i], positions[j], 1);
                    if (j < end_index && j >= start_index)
                    {
                    	//виключаємо малювання зв'язків при відображенні зв'язків одної вершини
                        connect_nodes(cr, positions[j], positions[i], -1);
                    }
                }
            }
            else if (directed || i <= j)
            {
                connect_nodes(cr, positions[i], positions[j], 
                j == (node_count - 1) && !directed ? 1 : 0);
            }
        }
    }
}

void draw_node(cairo_t *cr, node_pos_t pos, char *text)
{
    cairo_move_to(cr, pos.x + NODE_RADIUS, pos.y);
    cairo_arc(cr, pos.x, pos.y, NODE_RADIUS, 0, 2 * M_PI); //малюємо еліпс
    cairo_stroke_preserve(cr);

    cairo_set_font_size(cr, NODE_RADIUS);
    if (strlen(text) > 1) {
        cairo_move_to(cr, pos.x - NODE_RADIUS / 2, pos.y + NODE_RADIUS / 3);
    } else {
        cairo_move_to(cr, pos.x - NODE_RADIUS / 3.5, pos.y + NODE_RADIUS / 3);
    }
    
    cairo_show_text(cr, text);
}

void draw_nodes(cairo_t *cr, node_pos_t *positions)
{
    for (int i = 1; i <= node_count; i++)
    {
        char text[3];
        sprintf(text, "%d", i);

        draw_node(cr, positions[i - 1], text);
    }
}

void draw_graph(cairo_t *cr, double **matrix, node_pos_t *positions)
{
    cairo_set_source_rgb(cr, 0.117, 0.631, 0.760); //встановлюємо забарвлення

    draw_nodes(cr, positions);
    draw_connections(cr, positions, matrix);

    free(positions);
}

void set_window_size()
{
   	//враховуємо відступи і радіус вершин
    if (node_count > 5) {   
        window_height = 2 * MARGIN + window_field.right * NODE_RADIUS * 2 + NODE_SPACING * (window_field.right - 1);
        window_width = 2 * MARGIN + window_field.bottom * NODE_RADIUS * 2 + NODE_SPACING * (window_field.bottom - 1);
    } 
    else 
    {
        window_width = 2 * MARGIN + node_count * NODE_RADIUS * 2 + NODE_SPACING * (node_count - 1);
        window_height = 2 * MARGIN + NODE_RADIUS * 2;
    }
}

void calculate_size()
{
    int free_count = node_count - 4 - 1; //віднімаємо кількість крайніх вершин і вершину в центрі
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
    node_pos_t *positions = get_node_positions();
    draw_graph(cr, matrix, positions);
    return FALSE;
}

GtkWidget *create_window(GtkApplication *app)
{
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), directed ? APP_NAME_DIRECTED : APP_NAME_UNDIRECTED); //встановлюємо ім'я програми
    gtk_window_set_default_size(GTK_WINDOW(window), window_width, window_height); //встановлюємо розмір вікна

    return window;
}

GtkWidget *create_darea(GtkWidget *window)
{
    GtkWidget *darea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), darea);

   	//призначаємо колбек для рисування об'єктів
    g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), NULL);
   	//призначаємо колбек, який викликається при виході з програми 
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    return darea;
}

void on_app_activate(GtkApplication *app, gpointer data)
{
    GtkWidget *window = create_window(app);
   	//створюється площина для малювання, яка прив'язується до вікна
    GtkWidget *darea = create_darea(window);

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

void directed_read()
{
    printf("Output directed graph? (0 for no, any other number for yes)\n");
    scanf("%d", &directed);
    directed = !directed ? 0 : 1;
}

void node_read()
{
    node_shown = -2;
    while (node_shown < -1 || node_shown >= node_count)
    {
        printf("What node connections to show? (input index of node or -1 to show all node connections)\n");
        scanf("%d", &node_shown);
    }
}

int type_read() {
    int type = 0; //тип матриці задає коефіцієнт, на який вона множиться після випадкової генерації
    while (type != 1 && type != 2) 
    {
        printf("Input type of matrix:\n");
        scanf("%d", &type);
    }
    return type;
}

int condensed_read() {
    int condensed; //визначаємо, чи відображати граф конденсації замість звичайного
    printf("Condense the graph?\n");
    scanf("%d", &condensed);
    return !condensed ? 0 : 1;
}

int main(int argc, char *argv[])
{
    directed_read();
    node_read();
    int type = type_read();
    int condensed = 0;
    if (directed) {
        condensed = condensed_read();
    }

    matrix = get_matrix(type);
    node_count = NODE_COUNT;
    if (!directed) 
    {
        to_undirected(matrix);
    }

    printf("\nOriginal matrix:\n");
    output_matrix(node_count, node_count, matrix);

    if (condensed) 
    {
        double **temp = matrix;
        matrix = get_condensed_matrix(node_count, matrix);
        free_matrix(node_count, temp);
        node_count = get_condensed_matrix_size();
    }

    set_directed(directed);
    set_count(node_count);

    if (!condensed) additional_output(type, matrix);

    calculate_size();
    create_application(argc, argv);

    free_matrix(node_count, matrix);

    return 1;
}