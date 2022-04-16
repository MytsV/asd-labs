#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <cairo.h>
#include <math.h>
#include "../headers/matrix_tools.h"
#include "../headers/data_structures.h"

#define UNUSED_FLAG 0
#define ACTIVE_FLAG 1
#define USED_FLAG 2
#define SKIPPED_FLAG 3
#define ENQUEUED_FLAG 4

const char APP_NAME[] = "Graph Search";

/*
КОНСТАНТИ РОЗМІРІВ ГРАФУ
*/

const int MARGIN = 115;	//відступи по краях
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

const int CONNECTION_OFFSET = 7; //зміщення ліній при з'єднанні двох вершин двічі

const int ARROW_LENGTH = 18;
const double ARROW_ANGLE = M_PI / 6;

const double CURVE_ARROW_ANGLE_X = M_PI / 3;
const double CURVE_ARROW_ANGLE_Y = M_PI / 4;

const double WINDOW_HEIGHT_OFFSET_SIZE = 0.6; //частина вікна (у відношенні), в якій деякі зв'язки будуть зі зміщенням вверх

/*
ПАРАМЕТРИ ВІДОБРАЖННЯ НИЖНЬОЇ КНОПКИ
*/

const int BUTTON_HEIGHT = 100;
const int BUTTON_MARGIN = 20;
const int BUTTON_ROW = 7;

/*
ПАРАМЕТРИ ВІДОБРАЖЕННЯ ЛЕГЕНДИ
*/

const int LEGEND_RADIUS = 20;
const int LEGEND_MARGIN = 20;
const int SPACING_LEGEND = 150;

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

typedef struct color //тут значення RGB є нормованими, тобто діляться на 0xFF
{
    double r;
    double g;
    double b;
}
color_t;

//індекс кольору в масиві відповідний значенню константи вище
color_t colors[] = {{0.792, 0.976, 0.764}, {0.964, 0.717, 0.737}, {0.976, 0.933, 0.823}, {0.870, 0.870, 0.870}, {0.858, 0.952, 0.980}};
color_t colors_border[] = {{0.572, 0.945, 0.517}, {0.913, 0.427, 0.466}, {0.925, 0.811, 0.513}, {0.870, 0.870, 0.870}, {0.584, 0.850, 0.937}};

int window_width;
int window_height;

struct field window_field;

double **matrix;
double **tree_matrix;

my_stack_t *element_stack; //стек вершин, призначений для пошуку в глибину
my_queue_t *element_queue; //черга вершин, призначена для пошуку в ширину

int *neighbour_indices; //масив, у якому зберігається індекс останнього проглянутого сусіда

int is_dfs;
int search_end = 0; //параметр, що зазначає, чи закінчився пошук
int show_tree = 0; //параметр, що зазначає, чи показується дерево

//ми окремо задаватимемо ці параметри, не використовуючи константи в коді, оскільки для відображення дерева їх потрібно буде змінювати
int node_radius; 
int node_spacing;
int margin;

double **connection_status; //матриця, яка містить прапорець для кожного з'єднання (USED_FLAG, ACTIVE_FLAG...)
int *node_status; //масив зі станами вершин (USED_FLAG, ACTIVE_FLAG...)
 
void draw_arrow(cairo_t *cr, double x, double y, double angle)
{
   	//розраховуємо кути, під якими розташовані бокові лінії стрілки у відношенні до осі x
    double g1 = (M_PI / 2 - angle) - ARROW_ANGLE;
    double g2 = ARROW_ANGLE - angle;

    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x - ARROW_LENGTH * cos(g1), y - ARROW_LENGTH * sin(g1));
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x + ARROW_LENGTH * sin(g2), y - ARROW_LENGTH * cos(g2));
}

double get_tilted_arrow_angle(double dx, double dy, int x_sign, int y_sign)
{
    double tangent = (double) dx / dy;

    double angle = 0;
    if (x_sign == 1)
    {
        if (y_sign == 1)
        {
            angle = atan(tangent);
        }
        else
        {
            angle = M_PI - atan(tangent);
        }
    }
    else
    {
        if (y_sign == 1)
        {
            angle = -atan(tangent);
        }
        else
        {
            angle = M_PI + atan(tangent);
        }
    }

    return angle;
}

void connect_with_self(cairo_t *cr, node_pos_t node_n)
{
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

    draw_arrow(cr, end_x, end_y, y_offset_sign == -1 ? 0 : M_PI);
}

void connect_horizontal(cairo_t *cr, node_pos_t node_n, node_pos_t node_m, double offset)
{
    double dx_signed = node_m.x - node_n.x;
    int x_sign = dx_signed > 0 ? 1 : -1;
    double dx = fabs(dx_signed);

   	//якщо вершини не є сусідніми, ми проведемо між ними криву
    if (dx > node_spacing * 2)
    {
        int y_offset_sign = node_n.y < window_height * WINDOW_HEIGHT_OFFSET_SIZE ? -1 : 1;

        double y_margin = y_offset_sign * CURVE_HOISTING; //піднімаємо криву від центра кола, зміщення завжди буде вгору
        double x_margin = sqrt(node_radius * node_radius - y_margin * y_margin); //розраховуємо зміщення по x за допомогою теореми Піфагора
        x_margin *= x_sign;	//домножуємо зміщення лише по x на знак різниці координат

        double start_x = node_n.x + x_margin;
        double start_y = node_n.y + y_margin;

        double middle_x = node_n.x + x_sign * dx / 2; //середина кривої буде розміщуватися рівно між вершинами по x
        double middle_y = node_n.y + y_offset_sign * node_spacing + y_margin + 2 * offset; //і буде піднята на висоту стандартної відстані між вершинами

        double end_x = node_m.x - x_margin;
        double end_y = node_m.y + y_margin;

        cairo_move_to(cr, start_x, start_y);
        cairo_curve_to(cr, start_x, start_y, middle_x, middle_y, end_x, end_y);

        double angle = x_sign * CURVE_ARROW_ANGLE_X;
        draw_arrow(cr, end_x, end_y, y_offset_sign == -1 ? angle : M_PI - angle);
    }
    else
    {
        double y_margin = offset;
        double x_margin = sqrt(node_radius * node_radius - y_margin * y_margin) * x_sign;

        double end_x = node_m.x - x_margin;
        double end_y = node_m.y + y_margin;

        cairo_move_to(cr, node_n.x + x_margin, node_n.y + y_margin);
        cairo_line_to(cr, end_x, end_y);

        draw_arrow(cr, end_x, end_y, x_sign * M_PI / 2);
    }
}

void connect_vertical(cairo_t *cr, node_pos_t node_n, node_pos_t node_m, double offset)
{
    double dy_signed = node_m.y - node_n.y;
    int y_sign = dy_signed > 0 ? 1 : -1;
    double dy = fabs(dy_signed);

   	//якщо вершини не є сусідніми, ми проведемо між ними криву
    if (dy > node_spacing * 2)
    {
        double x_margin = -CURVE_HOISTING; //зсуваємо криву від центра кола вліво
        double y_margin = sqrt(node_radius * node_radius - x_margin * x_margin);	//розраховуємо зміщення по y за допомогою теореми Піфагора
        y_margin *= y_sign;	//домножуємо зміщення лише по x на знак різниці координат

        double start_x = node_n.x + x_margin;
        double start_y = node_n.y + y_margin;

        double middle_x = node_n.x - node_spacing * 2 - x_margin + 2 * offset; //і буде зміщена вліво на стандартну відстань між вершинами
        double middle_y = node_n.y + y_sign * dy / 2; //середина кривої буде розміщуватися рівно між вершинами по y

        double end_x = node_m.x + x_margin;
        double end_y = node_m.y - y_margin;

        cairo_move_to(cr, start_x, start_y);
        cairo_curve_to(cr, start_x, start_y, middle_x, middle_y, end_x, end_y);

        draw_arrow(cr, end_x, end_y, y_sign == 1 ? CURVE_ARROW_ANGLE_Y : M_PI - CURVE_ARROW_ANGLE_Y);
    }
    else
    {
        double x_margin = offset;
        double y_margin = sqrt(node_radius * node_radius - x_margin * x_margin) * y_sign;

        double end_x = node_m.x + x_margin;
        double end_y = node_m.y - y_margin;

        cairo_move_to(cr, node_n.x + x_margin, node_n.y + y_margin);
        cairo_line_to(cr, end_x, end_y);

        draw_arrow(cr, end_x, end_y, y_sign == 1 ? 0 : M_PI);
    }
}

void connect_tilted(cairo_t *cr, node_pos_t node_n, node_pos_t node_m, double offset)
{
   	//різниця між координатами зі знаком
    double dx_signed = node_m.x - node_n.x;
    double dy_signed = node_m.y - node_n.y;

    double dx = fabs(dx_signed);
    double dy = fabs(dy_signed);

    int x_sign = dx_signed > 0 ? 1 : -1; //зберігаємо знак, аби правильно розрахувати зміщення
    int y_sign = dy_signed > 0 ? 1 : -1;

   	//нахил до осі y
    double tangent = (double) dx / dy;

   	//розрахунок зміщення від центру кола до його краю за теоремою Піфагора
    double y_margin = sqrt((node_radius * node_radius) / (1 + tangent * tangent));
    double x_margin = y_margin * tangent;

    y_margin *= y_sign;
    x_margin *= x_sign;

    double start_x = node_n.x + x_margin;
    double start_y = node_n.y + y_margin;

    double middle_x = node_n.x + x_sign * dx / 2 + 8 * offset;
    double middle_y = node_n.y + y_sign * dy / 2;

   	//зберігаємо координати кінця задля перевикористання їх при малюванні стрілки
    double end_x = node_m.x - x_margin;
    double end_y = node_m.y - y_margin;

    cairo_move_to(cr, start_x, start_y);
    cairo_line_to(cr, middle_x, middle_y);
    cairo_line_to(cr, end_x, end_y);

    draw_arrow(cr, end_x, end_y, get_tilted_arrow_angle(fabs(middle_x - end_x), fabs(middle_y - end_y), x_sign, y_sign));
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

node_pos_t *get_node_positions()
{
    node_pos_t *positions = malloc(sizeof(node_pos_t) * NODE_COUNT);

    int index = 0;
    set_side_positions(positions, window_field.top, &index, get_top_position);

    set_side_positions(positions, window_field.right - 1, &index, get_right_position);

    set_side_positions(positions, window_field.bottom - 1, &index, get_bottom_position);

    set_side_positions(positions, window_field.left - 2, &index, get_left_position);

    set_side_positions(positions, 1, &index, get_center_position);

    return positions;
}

node_pos_t *get_node_positions_tree() 
{
    node_pos_t *positions = malloc(sizeof(node_pos_t) * NODE_COUNT);

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

    return positions;
}

void set_node_border(cairo_t *cr, int status) 
{
    color_t color = colors_border[status];
    cairo_set_source_rgb(cr, color.r, color.g, color.b);
}

void draw_connections(cairo_t *cr, node_pos_t *positions, double **matrix)
{
    for (int i = 0; i < NODE_COUNT; i++)
    {
        for (int j = 0; j < NODE_COUNT; j++)
        {
            if (!matrix[i][j]) continue; //якщо в матриці на позиції стоїть нуль, пропускаємо з'єднання

            set_node_border(cr, connection_status[i][j]);

            if (i != j && matrix[j][i] == 1)
            {
            	//перевіряємо, чи є подвійне з'єднання вершин
                if (i < j)
                {
                    connect_nodes(cr, positions[i], positions[j], CONNECTION_OFFSET);
                    set_node_border(cr, connection_status[j][i]);
                    connect_nodes(cr, positions[j], positions[i], -CONNECTION_OFFSET);
                }
            }
            else
            {
                connect_nodes(cr, positions[i], positions[j], 0);
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

void draw_legend_node(cairo_t *cr, node_pos_t pos, int flag, char *text)
{
    cairo_move_to(cr, pos.x + LEGEND_RADIUS, pos.y);
    set_node_border(cr, flag);
    cairo_arc(cr, pos.x, pos.y, LEGEND_RADIUS, 0, 2 * M_PI); //малюємо еліпс
    cairo_stroke_preserve(cr);
    set_node_color(cr, flag);

    set_node_border(cr, flag);
    cairo_set_font_size(cr, LEGEND_RADIUS / 2);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_move_to(cr, pos.x + LEGEND_RADIUS + LEGEND_RADIUS / 2, pos.y);
    cairo_show_text(cr, text);
}

char *get_legend_text(int status)
{
    char *text;
    switch(status) 
    {
        case UNUSED_FLAG: //не проглянутий
        {
            text = "Не проглянутий";
            break;
        }
        case ACTIVE_FLAG: //активний
        {
            text = "Активний";
            break;
        }
        case USED_FLAG: //проглянутий
        {
            text = "Проглянутий";
            break;
        }
        case ENQUEUED_FLAG: //в черзі
        {
            text = "У черзі";
            break;
        }
        case SKIPPED_FLAG:
        {
            text = "Пропущений";
            break;    
        }            
    }
    return text;
}

void draw_legend(cairo_t *cr)
{
    int spaced = 0;

    for (int i = UNUSED_FLAG; i <= (is_dfs ? SKIPPED_FLAG : ENQUEUED_FLAG); i++)
    {
        node_pos_t position = {LEGEND_MARGIN + LEGEND_RADIUS + spaced, LEGEND_MARGIN * 2};
        draw_legend_node(cr, position, i, get_legend_text(i));
        spaced += SPACING_LEGEND;
    }
}

void move_for_text(cairo_t *cr, node_pos_t pos, char *text)
{
    if (strlen(text) > 1) 
    {
        cairo_move_to(cr, pos.x - node_radius / 2, pos.y + node_radius / 3);
    } 
    else 
    {
        cairo_move_to(cr, pos.x - node_radius / 3.5, pos.y + node_radius / 3);
    }
}

void draw_node(cairo_t *cr, node_pos_t pos, char *text, int status)
{
    cairo_move_to(cr, pos.x + node_radius, pos.y);
    set_node_border(cr, status);
    cairo_arc(cr, pos.x, pos.y, node_radius, 0, 2 * M_PI); //малюємо еліпс

    cairo_stroke_preserve(cr);
    set_node_color(cr, status);

    set_node_border(cr, status);
    cairo_set_font_size(cr, node_radius);
    move_for_text(cr, pos, text);

    cairo_show_text(cr, text);
}

void draw_nodes(cairo_t *cr, node_pos_t *positions)
{
    for (int i = 1; i <= NODE_COUNT; i++)
    {
        char text[3];
        sprintf(text, "%d", i);

        draw_node(cr, positions[i - 1], text, node_status[i - 1]);
    }
}

void draw_graph(cairo_t *cr, double **matrix)
{
    node_pos_t *positions = show_tree ? get_node_positions_tree() : get_node_positions();

    draw_nodes(cr, positions);
    draw_connections(cr, positions, matrix);
    if (!show_tree)
    {
        draw_legend(cr);
    }

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

void dfs() //пошук в глибину
{
    int element = peek_stack(element_stack);
    node_status[element] = USED_FLAG;

    if (element_stack->last_index != 0) 
    {
        pop_stack(element_stack);
        connection_status[peek_stack(element_stack)][element] = USED_FLAG;
        tree_matrix[peek_stack(element_stack)][element] = 1;
        push_stack(element_stack, element);
    }

    int neighbour = -1;
    for (int i = neighbour_indices[element]; i < NODE_COUNT; i++) 
    {
        if (matrix[element][i])
        {
            if (node_status[i] != USED_FLAG)
            {
                neighbour = i;
                connection_status[element][i] = ACTIVE_FLAG;
                neighbour_indices[element] = i + 1;
                break;
            }
            else if (connection_status[element][i] != USED_FLAG)
            {
                connection_status[element][i] = SKIPPED_FLAG;
            }
        }
    }

    if (neighbour == -1)
    {
        pop_stack(element_stack);   
    }
    else 
    {
        push_stack(element_stack, neighbour);
    }

    node_status[peek_stack(element_stack)] = ACTIVE_FLAG;
}

void replace_connection_status(int element, int to_replace, int replacement)
{
    for (int i = 0; i < NODE_COUNT; i++)
    {
        if (connection_status[i][element] == to_replace)
        {
            connection_status[i][element] = replacement;
            if (!is_dfs && replacement == USED_FLAG)
            {
                tree_matrix[i][element] = 1;
            }
        }
    }
}

void bfs() //пошук в ширину
{
    int element = peek_queue(element_queue);

    node_status[element] = USED_FLAG;

    if (element_queue->front != 0)
    {
        replace_connection_status(element, ACTIVE_FLAG, USED_FLAG);
    }

    for (int i = 0; i < NODE_COUNT; i++) 
    {
        if (matrix[element][i])
        {
            if (node_status[i] != USED_FLAG && node_status[i] != ENQUEUED_FLAG)
            {
                enqueue_queue(element_queue, i);
                node_status[i] = ENQUEUED_FLAG;
                connection_status[element][i] = ENQUEUED_FLAG;
            }
            else if (connection_status[element][i] != USED_FLAG)
            {
                connection_status[element][i] = SKIPPED_FLAG;
            }
        }
    }

    dequeue_queue(element_queue);
    node_status[peek_queue(element_queue)] = ACTIVE_FLAG;
    replace_connection_status(peek_queue(element_queue), ENQUEUED_FLAG, ACTIVE_FLAG);
}

void on_search_end(GtkWidget *widget)
{
    printf("Search is complete\n");
    gtk_button_set_label(GTK_BUTTON(widget), "Показати дерево обходу");
    search_end = 1;
}

static void on_clicked(GtkWidget *widget, gpointer data)
{
    if (search_end)
    {
        printf("\nTree of graph search\n\n");
        output_matrix(NODE_COUNT, NODE_COUNT, tree_matrix);
        matrix = tree_matrix;
        
        gtk_widget_hide(widget);

        margin = MARGIN_TREE;
        node_radius = NODE_RADIUS_TREE;
        node_spacing = NODE_SPACING_TREE;
        show_tree = 1;
    } 
    else if (is_dfs)
    {
        if (element_stack->last_index != -1)
        {
            dfs();
        }
        else 
        {
            on_search_end(widget);
        }
    }
    else
    {
        if (element_queue->size != 0)
        {
            bfs();
        }
        else 
        {
            on_search_end(widget);
        }
    }
    
    gtk_widget_queue_draw(widget);
}

GtkWidget *create_window(GtkApplication *app)
{
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), APP_NAME); //встановлюємо ім'я програми
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

GtkWidget *create_grid(GtkWidget *darea, GtkWidget *button_next) {
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

void init_neighbour_indices()
{
    neighbour_indices = (int *)malloc(sizeof(int) * NODE_COUNT);
    for (int i = 0; i < NODE_COUNT; i++) {
        neighbour_indices[i] = 0;
    }
}

int get_first_node()
{
    int first_node = -1;

    for (int i = 0; i < NODE_COUNT; i++) 
    {
        int out = 0;
        for (int j = 0; j < NODE_COUNT; j++) 
        {
            if (matrix[i][j] && i != j) 
            {
                out = 1;
                break;
            }
        }
        if (out) 
        {
            first_node = i;
            break;
        }
    }

    return first_node;
}

void init_element_stack()
{
    element_stack = new_stack(NODE_COUNT);
    int first_node = get_first_node();
    
    if (first_node != -1)
    {
        push_stack(element_stack, first_node);
        node_status[first_node] = 1;
    }
    else
    {
        printf("Fatal error: no nodes with out edges found");
        exit(1);
    }
}

void init_element_queue()
{
    element_queue = new_queue(NODE_COUNT);
    int first_node = get_first_node();
    
    if (first_node != -1)
    {
        enqueue_queue(element_queue, first_node);
        node_status[first_node] = 1;
    }
    else
    {
        printf("Fatal error: no nodes with out edges found");
        exit(1);
    }
}

void init_connection_status() 
{
    connection_status = (double **)malloc(sizeof(double *) * NODE_COUNT);
    tree_matrix = (double **)malloc(sizeof(double *) * NODE_COUNT);
    for (int i = 0; i < NODE_COUNT; i++) 
    {
        connection_status[i] = (double *)malloc(sizeof(double) * NODE_COUNT);
        tree_matrix[i] = (double *)malloc(sizeof(double) * NODE_COUNT);
        for (int j = 0; j < NODE_COUNT; j++) 
        {
            connection_status[i][j] = 0;
            tree_matrix[i][j] = 0;
        }
    }
}

void init_node_status() 
{
    node_status = (int *)malloc(sizeof(int) * NODE_COUNT);
    for (int i = 0; i < NODE_COUNT; i++) 
    {
        node_status[i] = 0;
    }
}

void init_search_values()
{
    init_connection_status();
    init_node_status();
    init_element_stack();
    init_element_queue();
    init_neighbour_indices();
}

void init_size_values()
{
    margin = MARGIN;
    node_radius = NODE_RADIUS;
    node_spacing = NODE_SPACING;
}

void init_is_dfs()
{
    is_dfs = -1;
    while (is_dfs != 0 && is_dfs != 1) 
    {
        printf("Use dfs search? (1 for yes, 0 for no)");
        scanf("%d", &is_dfs);
    }
}

int main(int argc, char *argv[])
{
    init_is_dfs();
    init_size_values();

    calculate_size();

    matrix = get_matrix();
    output_matrix(NODE_COUNT, NODE_COUNT, matrix);

    init_search_values();

    create_application(argc, argv);

    free_matrix(NODE_COUNT, matrix);

    return 1;
}