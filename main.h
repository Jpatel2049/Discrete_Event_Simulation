#ifndef MYFILE_H_
#define MYFILE_H_

/* Event Types */
#define ARRIVAL 0
#define FINISH 1
#define DISK_ARRIVAL 2
#define DISK_FINISH 3
#define SIMULATION_END 4

/* CPU/Disk states. */
#define IDLE 5
#define BUSY 6

/* Event Struct. */
typedef struct Event {
  int job_num;
  int type;
  int time;
}Event;

/* Queue Struct. */
typedef struct Queue {
  Event job;
  struct Queue *next;
}Queue;

/* Function Declarations */
int my_random(int low, int high);
char* get_value(char* file, char* str);
Queue* create_queue();
Queue* create_p_queue();
Event new_event(int num, int type, int time);
Event pop(Queue** head);
void p_push(Queue** head, Event new_event, int size);
void delete_queue(Queue** head);
Queue* print_queue(Queue** head);
void push(Queue** head, Event new_event, int size);
void process_CPU(Event curr);
void process_DISK(Event curr);
void find_max();
void print_stats();

#endif
