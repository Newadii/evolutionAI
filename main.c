#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#define GENERATION_SIZE 100

const unsigned int x_dim, y_dim;
static char **map;

typedef struct ROCK_POS {
    int x;
    int y;
    struct ROCK_POS *next;
} rock_pos;
rock_pos *first;
int rock_count;

typedef struct UNIT_GENE {
    char *in_pos; //in posiotion
    char *colision; //colision direction - 1: up/right 0: down/left
} unit_gene;
int gene_size;

void conf_init(int argc, char **argv)
{
    if (argc <= 1)
    {
        printf("no map configuration file!");
        exit(EXIT_FAILURE);
    }
    FILE *fp = fopen(argv[1], "r");
    if (!fp)
    {
        printf("can't open configuration file!");
        exit(EXIT_FAILURE);
    }
    rock_pos *rp, *tmp = NULL;
    rp = first = malloc(sizeof(rock_pos));
    rp->next = NULL;

    fscanf(fp, "%d%d", &x_dim, &y_dim);

    while (fscanf(fp, "%d%d", &(rp->x), &(rp->y)) != -1)
    {
        rock_count++;
        tmp = rp;
        rp->next = malloc(sizeof(rock_pos));
        rp = rp->next;
        rp->next = NULL;
    }
    free(rp);
    if (tmp)
        tmp->next = NULL;
    fclose(fp);
    gene_size = x_dim + y_dim + rock_count;
}

void make_map()
{
    if (!map)
    {
        map = malloc((x_dim+1) * sizeof(char *));
        for (int i=0; i<x_dim+1; i++)
        {
            map[i] = malloc((y_dim+1) * sizeof(char));
        }
    }
    for (int i=0; i<x_dim; i++)
    {
        memset(map[i], 0, y_dim);
        map[i][y_dim] = -2;
    }
    memset(map[x_dim], -2, y_dim);
    rock_pos *rp = first;
    int i=1;
    while (rp)
    {

        map[rp->x][rp->y] = i;
        rp = rp->next;
        i++;
    }
}

int crawl_map(int x, int y, int dir, int axys, unit_gene *g)
{
    int fitness=0;
    int tmp_dir;
    while (1)
    {
        if (x < 0 || y < 0 || map[x][y] == -2)
        {
            break;
        } else if (map[x][y] == 0)
        {
            fitness++;
            map[x][y] = -1;
            if (axys == 1)
                y += dir;
            else
                x += dir;
        } else if (map[x][y] > 0)
        {
            tmp_dir = g->colision[map[x][y]-1] == 1 ? 1 : -1;
            if (axys == 1)
                y -= dir;
            else
                x -= dir;
            dir = tmp_dir;
            axys ^= 1;
            if (axys == 1)
                y -= dir;
            else
                x -= dir;
        } else if (map[x][y] == -1)
        {
            if (axys == 1)
                y -= dir;
            else
                x -= dir;
            if (x < 0 || y < 0 || map[x][y] == -2)
                break;
            else if (map[x+1][y] > -1)
                x++;
            else if (x>1 && map[x-1][y] > -1)
                x--;
            else if (map[x][y+1] > -1)
                y++;
            else if (y>0 && map[x][y-1] > -1)
                y--;
            else
                break;
        } else
        {
            printf("something went wrong!\n");
            exit(EXIT_FAILURE);
        }
    }
    return fitness;
}

//returns fitness
int test_gene(unit_gene *g)
{
    int fitness = 0;
    int axys, dir, tmp_dir, x, y;
    make_map();
    for (int i=0; i<x_dim; i++)
    {
        x = i;
        y = g->in_pos[i] == 1 ? 0 : y_dim -1;
        dir = g->in_pos[i] == 1 ? 1 : -1;
        axys = 1;
        fitness += crawl_map(x, y, dir, axys, g);
    }

    for (int i=0; i<y_dim; i++)
    {
        x = g->in_pos[i + x_dim] == 1 ? 0 : x_dim -1;
        y = i;
        dir = g->in_pos[i + x_dim] == 1 ? 1 : -1;
        axys = 0;
        fitness += crawl_map(x, y, dir, axys, g);
    }
    return fitness;
}

void random_unit(unit_gene *g)
{
    time_t t;
    srand((unsigned) time(&t));

    for (int i=0; i < rock_count; i++)
        g->colision[i] = (char)(rand() & 1);
    for (int i=0; i < x_dim + y_dim; i++)
        g->in_pos[i] = (char)(rand() & 1);
}

unit_gene **create_random_generation()
{
    time_t t;
    srand((unsigned) time(&t));

    unit_gene **units = malloc(GENERATION_SIZE * sizeof(unit_gene *));
    for (int i=0; i<GENERATION_SIZE; i++)
    {
        units[i] = malloc(sizeof(unit_gene));
        units[i]->in_pos = malloc((x_dim + y_dim) * sizeof(char));
        units[i]->colision = malloc(rock_count * sizeof(char));
        for (int k=0; k < rock_count; k++)
            units[i]->colision[k] = (char)(rand() & 1);
        for (int k=0; k < x_dim + y_dim; k++)
            units[i]->in_pos[k] = (char)(rand() & 1);
    }
    return units;
}

void print_map()
{
    for (int i=0; i<x_dim; i++)
    {
        for (int k=0; k<y_dim; k++)
        {
            printf("%c ", map[i][k] + '0');
        }
        printf("\n");
    }
}

int main(int argc, char *argv[], char *envp[])
{
    conf_init(argc, argv);
    make_map();

    printf("%d,%d\n",y_dim,x_dim);
    rock_pos *rp = first;
    while (rp)
    {
        printf("x:%d y:%d\n", rp->x, rp->y);
        rp = rp->next;
    }
    unit_gene **units = create_random_generation();

    for (int i=0; i<GENERATION_SIZE; i++)
    {
        printf("unit%d fitness: %d\n", i, test_gene(units[i]));
    }
//    print_map();

    for (int i=0; i<x_dim; i++)
        free(map[i]);
    free(map);
    return 0;
}