#include "dispatcher.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

char	*itob(int value)
{
	unsigned int	i;
	char			*string;

	i = 0;
	string = calloc(1, sizeof(int));
	while (i < sizeof(int))
	{
		string[i] = ((char *)(&value))[i];
		i++;
	}
	return string;	
}

char	*ftob(float value)
{
	char *string;

	string = calloc(1, sizeof(float));
	for (unsigned int i = 0; i < sizeof(float); i++)
		string[i] = ((char *)(&value))[i];
	return string;	
}

void	strbjoin(t_serial *serial, char const *s2, size_t size)
{
	char	*copy;
	int		i;

	i = 0;
	copy = (char *)malloc(serial->len);
	for (int x = 0; x < serial->len; x++)
		copy[x] = serial->data[x];
	serial->data = (char *)calloc(1, serial->len + size);
	while (i < serial->len)
	{
		serial->data[i] = copy[i];
		i++;
	}
	for (unsigned int x = 0; x < size; x++)
	{
		serial->data[i] = s2[x];
		i++;
	}
	serial->len += size;
}

t_serial *work_unit_serializer(t_work_unit *work_unit)
{
	t_serial		*serial;

	serial = malloc(sizeof(serial));
	serial->data = itob(1);
	serial->len = 4;
	strbjoin(serial, itob(work_unit->compute_class), sizeof(int));
	strbjoin(serial, itob(work_unit->cell.body_count), sizeof(int));
	for (int i = 0; i< work_unit->cell.body_count; i++)
	{
		strbjoin(serial, ftob(work_unit->cell.contained_bodies[i]->position.x), sizeof(float));
		strbjoin(serial, ftob(work_unit->cell.contained_bodies[i]->position.y), sizeof(float));
		strbjoin(serial, ftob(work_unit->cell.contained_bodies[i]->position.z), sizeof(float));
		strbjoin(serial, ftob(work_unit->cell.contained_bodies[i]->velocity.x), sizeof(float));
		strbjoin(serial, ftob(work_unit->cell.contained_bodies[i]->velocity.y), sizeof(float));
		strbjoin(serial, ftob(work_unit->cell.contained_bodies[i]->velocity.z), sizeof(float));
		strbjoin(serial, ftob(work_unit->cell.contained_bodies[i]->mass), sizeof(float));
	}
	strbjoin(serial, ftob(work_unit->cell.cell_as_body.position.x), sizeof(float));
	strbjoin(serial, ftob(work_unit->cell.cell_as_body.position.y), sizeof(float));
	strbjoin(serial, ftob(work_unit->cell.cell_as_body.position.z), sizeof(float));
	strbjoin(serial, ftob(work_unit->cell.cell_as_body.velocity.x), sizeof(float));
	strbjoin(serial, ftob(work_unit->cell.cell_as_body.velocity.y), sizeof(float));
	strbjoin(serial, ftob(work_unit->cell.cell_as_body.velocity.z), sizeof(float));
	strbjoin(serial, ftob(work_unit->cell.cell_as_body.mass), sizeof(float));
	strbjoin(serial, itob(work_unit->adjoining_cells_cnt), sizeof(int));
	for (int i = 0; i < work_unit->adjoining_cells_cnt; i++)
	{
		strbjoin(serial, itob(work_unit->adjoining_cells[i].body_count), sizeof(int));
		for (int x = 0; x < work_unit->adjoining_cells[i].body_count; x++)
		{
			strbjoin(serial, ftob(work_unit->adjoining_cells[i].contained_bodies[x]->position.x), sizeof(float));
			strbjoin(serial, ftob(work_unit->adjoining_cells[i].contained_bodies[x]->position.y), sizeof(float));
			strbjoin(serial, ftob(work_unit->adjoining_cells[i].contained_bodies[x]->position.z), sizeof(float));
			strbjoin(serial, ftob(work_unit->adjoining_cells[i].contained_bodies[x]->velocity.x), sizeof(float));
			strbjoin(serial, ftob(work_unit->adjoining_cells[i].contained_bodies[x]->velocity.y), sizeof(float));
			strbjoin(serial, ftob(work_unit->adjoining_cells[i].contained_bodies[x]->velocity.z), sizeof(float));
			strbjoin(serial, ftob(work_unit->adjoining_cells[i].contained_bodies[x]->mass), sizeof(float));
		}
		strbjoin(serial, ftob(work_unit->adjoining_cells[i].cell_as_body.position.x), sizeof(float));
		strbjoin(serial, ftob(work_unit->adjoining_cells[i].cell_as_body.position.y), sizeof(float));
		strbjoin(serial, ftob(work_unit->adjoining_cells[i].cell_as_body.position.z), sizeof(float));
		strbjoin(serial, ftob(work_unit->adjoining_cells[i].cell_as_body.velocity.x), sizeof(float));
		strbjoin(serial, ftob(work_unit->adjoining_cells[i].cell_as_body.velocity.y), sizeof(float));
		strbjoin(serial, ftob(work_unit->adjoining_cells[i].cell_as_body.velocity.z), sizeof(float));
		strbjoin(serial, ftob(work_unit->adjoining_cells[i].cell_as_body.mass), sizeof(float));
	}
	return (serial);
}

#include <sys/types.h>
#include <sys/socket.h>

#define MYPORT "3490"  // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue will hold

int	send_message(t_serial *serial)
{
	int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[2000];
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
    //Listen
    listen(socket_desc , 3);
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    //accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");
    //Receive a message from client
    while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
    {
    	puts("loop");
    	printf("strlen: %d\n", serial->len);
        write(client_sock, serial->data, serial->len);
    }
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
        perror("recv failed");
    return 0;
}

int		main(void)
{
	int			i;
	t_work_unit	*work_unit;
	t_serial		*serial;

	i = -1;
	work_unit = malloc(sizeof(t_work_unit));
	work_unit->compute_class = 15;
	work_unit->cell.body_count = 15;
	work_unit->cell.contained_bodies = (t_body **)malloc(sizeof(t_body *) * work_unit->cell.body_count);
	for (int i = 0; i < work_unit->cell.body_count; i++)
	{
		work_unit->cell.contained_bodies[i] = (t_body *)malloc(sizeof(t_body));
		work_unit->cell.contained_bodies[i]->position.x = 123456789.123;
		work_unit->cell.contained_bodies[i]->position.y = 987654321.321;
		work_unit->cell.contained_bodies[i]->position.z = 123456789.123;
		work_unit->cell.contained_bodies[i]->velocity.x = 111111111.222;
		work_unit->cell.contained_bodies[i]->velocity.y = 222222222.333;
		work_unit->cell.contained_bodies[i]->velocity.z = 333333333.444;
		work_unit->cell.contained_bodies[i]->mass = 333333333.444;
	}
	work_unit->cell.cell_as_body.position.x = 55;
	work_unit->cell.cell_as_body.position.y = 55;
	work_unit->cell.cell_as_body.position.z = 55;
	work_unit->cell.cell_as_body.velocity.x = 22;
	work_unit->cell.cell_as_body.velocity.y = 22;
	work_unit->cell.cell_as_body.velocity.z = 22;
	work_unit->cell.cell_as_body.mass = 100;
	//.........adjoining cells........//
	work_unit->adjoining_cells = malloc(sizeof(t_cell));
	work_unit->adjoining_cells_cnt = 10;
	work_unit->adjoining_cells = (t_cell *)malloc(sizeof(t_cell) * work_unit->adjoining_cells_cnt);
	for (int i = 0; i < work_unit->adjoining_cells_cnt; i++)
	{
		work_unit->adjoining_cells[i].body_count = 10;
		work_unit->adjoining_cells[i].contained_bodies =
			(t_body **)malloc(sizeof(t_body *) * work_unit->adjoining_cells[i].body_count);
		for (int x = 0; x < work_unit->adjoining_cells[i].body_count; x++)
		{
			work_unit->adjoining_cells[i].contained_bodies[x] = (t_body *)malloc(sizeof(t_body));
			work_unit->adjoining_cells[i].contained_bodies[x]->position.x = i * x;
			work_unit->adjoining_cells[i].contained_bodies[x]->position.y = i * x;
			work_unit->adjoining_cells[i].contained_bodies[x]->position.z = i * x;
			work_unit->adjoining_cells[i].contained_bodies[x]->velocity.x = i * x;
			work_unit->adjoining_cells[i].contained_bodies[x]->velocity.y = i * x;
			work_unit->adjoining_cells[i].contained_bodies[x]->velocity.z = i * x;
			work_unit->adjoining_cells[i].contained_bodies[x]->mass = i * x;
		}
		work_unit->adjoining_cells->cell_as_body.position.x = i;
		work_unit->adjoining_cells->cell_as_body.position.y = i;
		work_unit->adjoining_cells->cell_as_body.position.z = i;
		work_unit->adjoining_cells->cell_as_body.velocity.x = i;
		work_unit->adjoining_cells->cell_as_body.velocity.y = i;
		work_unit->adjoining_cells->cell_as_body.velocity.z = i;
		work_unit->adjoining_cells->cell_as_body.mass = i;
	}
	serial = work_unit_serializer(work_unit);
	send_message(serial);
	//serializer_identifier(serial_str);
	return (0);
}