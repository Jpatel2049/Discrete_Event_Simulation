#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

/* Initializes the variables that we will read in. */
int SEED = 0;
int INIT_TIME = 0;
int	FIN_TIME = 0;
int ARRIVE_MIN = 0;
int ARRIVE_MAX = 0;
float QUIT_PROB = 0.0;
int CPU_MIN = 0;
int CPU_MAX = 0;
int DISK1_MIN = 0;
int DISK1_MAX = 0;
int DISK2_MIN = 0;
int DISK2_MAX = 0;

/* Total events in each queue. */
int p_total = 0, cpu_total = 0, disk1_total = 0, disk2_total = 0;

/* Times. */
int curr_time = 0, rand_time = 0;

/* Number of jobs. */
int job_num = 0;

/* Max queue sizes. */
int p_max = 0, cpu_max = 0, disk1_max = 0, disk2_max = 0;

/* Number of iterations. */
int total = 0;

/* Average sizes of queues. */
float p_avg = 0, cpu_avg = 0, disk1_avg = 0, disk2_avg = 0;

/* Total time server is busy. */
float cpu_time = 0, disk1_time = 0, disk2_time = 0;

/* Times servers become busy. */
int cpu_old = 0, disk1_old = 0, disk2_old = 0;

/* Total time. */
float total_time = 0;

/* Amount of jobs handled by each component. */
int cpu_job = 0, disk1_job = 0, disk2_job = 0;

/* Max time for jobs. */
int cpu_job_max = 0, disk1_job_max = 0, disk2_job_max = 0;

/* Queues */
Queue* p_queue = NULL;
Queue* cpu_queue = NULL;
Queue* disk1_queue = NULL;
Queue* disk2_queue = NULL;

/* Current event. */
Event curr;

/* Job numbers of Events in disks. */
int disk1, disk2;

/* States of CPU and disks. */
int cpu_state = IDLE, disk1_state = IDLE, disk2_state = IDLE;

/* File pointer to log file. */
FILE* fp;

/* Tests the quit probability. */
float t = 0, f = 0;

/* Test queues. */
Queue* test_p_queue = NULL;
Queue* test_queue = NULL;

int main() {
  /* Get the values from the text file. */
  SEED = atoi(get_value("input.txt", "SEED"));
  INIT_TIME = atoi(get_value("input.txt","INIT_TIME"));
  FIN_TIME = atoi(get_value("input.txt","FIN_TIME"));
  ARRIVE_MIN = atoi(get_value("input.txt","ARRIVE_MIN"));
  ARRIVE_MAX = atoi(get_value("input.txt","ARRIVE_MAX"));
  QUIT_PROB = atof(get_value("input.txt","QUIT_PROB"));
  CPU_MIN = atoi(get_value("input.txt","CPU_MIN"));
  CPU_MAX = atoi(get_value("input.txt","CPU_MAX"));
  DISK1_MIN = atoi(get_value("input.txt","DISK1_MIN"));
  DISK1_MAX = atoi(get_value("input.txt","DISK1_MAX"));
  DISK2_MIN = atoi(get_value("input.txt","DISK2_MIN"));
  DISK2_MAX = atoi(get_value("input.txt","DISK2_MAX"));

  /* Intialize the random number generator. */
  srand(SEED);
  /* Sets the current time to INIT_TIME. */
  curr_time = INIT_TIME;
  total_time = FIN_TIME - INIT_TIME;
  /* Opens the log file. */
  fp = fopen("Log.txt", "w");
  if(fp == NULL) {
    fprintf(stderr, "WARNING: Unable to open file.\n");
    exit(1);
  }

  /* Creates the queues. */
  p_queue = create_p_queue();
  cpu_queue = create_queue();
  disk1_queue = create_queue();
  disk2_queue = create_queue();

  /* Loop will continue to iterate until we reach SIMULATION_END, p_total
      should never be 0. */
  while(p_total != 0 && curr_time < FIN_TIME) {
    /* Removes first event from priority queue. */
    curr = pop(&p_queue);
    p_total--;
    /* Sets the current time to the current event's time. */
    curr_time = curr.time;
    /* Event Handler. */
    switch(curr.type) {
      case ARRIVAL: process_CPU(curr); break;
			case FINISH: process_CPU(curr); break;
			case DISK_ARRIVAL: process_DISK(curr); break;
			case DISK_FINISH: process_DISK(curr); break;
			case SIMULATION_END: break;
    }
    total++;
    p_avg += p_total;
    cpu_avg += cpu_total;
    disk1_avg += disk1_total;
    disk2_avg += disk2_total;
    find_max();
  }

  /* Add events to priority test queue. */
  int temp = 0;
  test_p_queue = (Queue *) malloc(sizeof(Queue));
  p_push(&test_p_queue, new_event(0, ARRIVAL, INIT_TIME), temp);
  temp++;
  p_push(&test_p_queue, new_event(0, SIMULATION_END, FIN_TIME), temp);
  temp++;
  p_push(&test_p_queue, new_event(5, FINISH, 45), temp);
  temp++;
  p_push(&test_p_queue, new_event(15, DISK_ARRIVAL, 23), temp);
  temp++;
  p_push(&test_p_queue, new_event(11, DISK_FINISH, 56), temp);
  temp++;
  p_push(&test_p_queue, new_event(9, ARRIVAL, 500), temp);
  temp++;

  /* Add events to FIFO test queue. */
  temp = 0;
  test_queue = (Queue *) malloc(sizeof(Queue));
  push(&test_queue, new_event(5, FINISH, 45), temp);
  temp++;
  push(&test_queue, new_event(15, DISK_ARRIVAL, 1), temp);
  temp++;
  push(&test_queue, new_event(11, DISK_FINISH, 56), temp);
  temp++;
  push(&test_queue, new_event(9, ARRIVAL, 5), temp);
  temp++;

  /* Prints out the statistics. */
  print_stats();
  /* Prints QUIT_PROB vs actual quit probability. */
  printf("Quit Probability %f Actual Quit Probability %f\n", QUIT_PROB, (f/(t+f)));
  /* Prints out the test_p_queue. */
  printf("Priority queue test.\n");
  print_queue(&test_p_queue);
  /* Prints out the test_queue. */
  printf("FIFO queue test.\n");
  print_queue(&test_queue);
  /* Deletes all of the queues. */
  delete_queue(&p_queue);
  delete_queue(&cpu_queue);
  delete_queue(&disk1_queue);
  delete_queue(&disk2_queue);
  delete_queue(&test_p_queue);
  delete_queue(&test_queue);
  /* Closes the file pointer to the log file. */
  fclose(fp);
  return 0;
}

/* Returns a random int from range low to high. */
int my_random(int low, int high) {
  return (low + random() % (high - low + 1));
}

/* Opens a file and returns the value associated with the input string. */
char* get_value(char* name, char* str) {
  int spce = 0;
  char line[20];
  char * output;
  FILE *fp = fopen(name, "r+");
  if(!fp) {
    printf("File not found.\n");
    return NULL;
  } else {
    while ((fgets(line, 20, fp)) != NULL) {
      if(strstr(line, str)) {
        char* temp = strtok(line, " ");
        temp = strtok(NULL, line);
        output = (char *) malloc((strlen(temp) + 1) * sizeof(char));
        strcpy(output, temp);
      }
    }
  }
  return output;
}

/* Creates the priority queue and pushes the first job and SIMULATION_END events into the queue. */
Queue* create_p_queue() {
  Queue* output = (Queue *) malloc(sizeof(Queue));
  Event init = new_event(++job_num, ARRIVAL, INIT_TIME);
  Event end = new_event(0, SIMULATION_END, FIN_TIME);
  p_push(&output, init, p_total);
  p_total++;
  p_push(&output, end, p_total);
  p_total++;
  return output;
}

/* Creates the FIFO queue. */
Queue* create_queue() {
  Queue* output = (Queue *) malloc(sizeof(Queue));
  return output;
}

/* Creates a new event. */
Event new_event(int num, int type, int time) {
  Event output;
  output.job_num = num;
  output.type = type;
  output.time = time;
  return output;
}

/* Removes the first event from the queue. */
Event pop(Queue** head) {
  Queue *temp = *head;
  *head = (*head)->next;
  Event output = temp->job;
  free(temp);
  return output;
}

/* Pushes a new event to the priority queue. */
void p_push(Queue** head, Event new_event, int size) {
  Queue *temp = (Queue *) malloc(sizeof(Queue));
  temp->job = new_event;
  temp->next = NULL;
  Queue* start = (*head);

  /* If the queue is empty. */
  if(size == 0) {
    *head = temp;
  /* If the first event of the queue has a greater time than the new event. */
  } else if((*head)->job.time > new_event.time) {
    temp->next = *head;
    (*head) = temp;
  /* Goes through the list until we find an event that has a greater time than the new event. */
  } else {
    while(start->next != NULL && start->next->job.time < new_event.time) {
      start = start->next;
    }
    temp->next = start->next;
    start->next = temp;
  }
}

/* Deletes the queue. */
void delete_queue(Queue** head) {
  while((*head) != NULL) {
    pop(head);
  }
}

/* Prints out the queue to standard out for testing purposes. */
Queue* print_queue(Queue** head) {
  Queue* temp = *head;
  while((*head) != NULL) {
    Event temp = (*head)->job;
    int time = temp.time;
    int type = temp.type;
    int num = temp.job_num;
    printf("Time %d Type %d Job Num %d\n", time, type, num);
    (*head) = (*head)->next;
  }
  return temp;
}

/* Pushes a new event on the FIFO queue. */
void push(Queue** head, Event new_event, int size) {
  Queue *temp = (Queue *) malloc(sizeof(Queue));
  temp->job = new_event;
  temp->next = NULL;
  Queue *start = (*head);

  /* If the queue is empty. */
	if (size == 0) {
		*head = temp;
  /* Place the event at the end of the queue. */
	} else {
    while(start->next != NULL) {
      start = start->next;
    }
    start->next = temp;
  }
}

/* Handles the CPU related events. */
void process_CPU(Event curr) {
  /* If event type is cpu arrival. */
  if(curr.type == ARRIVAL) {
    fprintf(fp, "At %d Job %d arrived at the CPU.\n", curr.time, curr.job_num);
    /* Creates a new job and pushes it onto the priority queue. */
    rand_time = my_random(ARRIVE_MIN, ARRIVE_MAX) + curr_time;
    Event temp = new_event(++job_num, ARRIVAL, rand_time);
    p_push(&p_queue, temp, p_total);
    p_total++;
    /* Pushes the current job onto the cpu FIFO queue. */
		push(&cpu_queue, curr, cpu_total);
    cpu_total++;

    /* If the CPU is not currently busy. */
    if(cpu_state == IDLE) {
      /* Pop the first event from the CPU queue. */
      curr = pop(&cpu_queue);
      cpu_total--;
      /* Creates the CPU finish event for this job and pushes it onto the priority queue. */
      rand_time = my_random(CPU_MIN, CPU_MAX) + curr_time;
      temp = new_event(curr.job_num, FINISH, rand_time);
      p_push(&p_queue, temp, p_total++);
      p_total++;
      /* Sets the CPU state to busy. */
      cpu_state = BUSY;
      cpu_old = curr_time;
    }
  /* If event type is CPU finish. */
  } else {
      /* Sets the CPU state to idle. */
      cpu_state = IDLE;
      int time = curr_time - cpu_old;
      cpu_time += time;
      cpu_job++;
      if(time > cpu_job_max) {
        cpu_job_max = time;
      }
      /* Generates a random exit value from 0-1 and compares it to the QUIT_PROB.
          if it is less, then it exits, if greater continues onto the disk. */
      double exit = ((random())/(double)RAND_MAX);
      if(exit > QUIT_PROB) {
        Event temp = new_event(curr.job_num, DISK_ARRIVAL, curr_time);
  			p_push(&p_queue, temp, p_total);
        p_total++;
        fprintf(fp, "At %d Job %d finished at the CPU.\n", curr.time, curr.job_num);
        t++;
      } else {
         fprintf(fp, "At %d Job %d exited.\n", curr.time, curr.job_num);
         f++;
      }
  }
}

/* Handles the disk related events. */
void process_DISK(Event curr) {
  /* If event type is disk arrival. */
  if(curr.type == DISK_ARRIVAL) {
    int send = 0;
    /* If disk 1 has less events in its queue than disk 2, send to disk 1. */
    if(disk1_total < disk2_total) {
      send = 1;
    /* If disk 1 has more events in its queue than disk 2, send to disk 2. */
    } else if(disk1_total > disk2_total) {
      send = 2;
    /* If both disks have equal amount of events, select a random disk. */
    } else {
      send = (rand() > RAND_MAX/2) ? 1 : 2;
    }

    fprintf(fp, "At %d Job %d arrived at the Disk.\n", curr.time, curr.job_num);

    /* Disk 1. */
    if(send == 1) {
      /* Push the current event onto disk 1's FIFO queue. */
      push(&disk1_queue, curr, disk1_total);
      disk1_total++;
      /* If disk 1 is currently not busy. */
      if(disk1_state == IDLE) {
        /* Pops the first element from the disk queue. */
        Event job = pop(&disk1_queue);
        disk1_total--;
        /* Creates the disk 1 finish event and pushes it onto the priority queue. */
        rand_time = my_random(DISK1_MIN, DISK1_MAX) + curr_time;
        Event temp = new_event(job.job_num, DISK_FINISH, rand_time);
        p_push(&p_queue, temp, p_total);
        p_total++;
        /* Saves the job number of what is currently in disk 1. */
        disk1 = job.job_num;
        /* Sets the disk 1 state to busy. */
        disk1_state = BUSY;
        disk1_old = curr_time;
      }
    /* Disk 2. */
    } else if(send == 2) {
      /* Push the current event onto disk 2's FIFO queue. */
      push(&disk2_queue, curr, disk2_total);
      disk2_total++;
      /* If disk 2 is currently not busy. */
      if(disk2_state == IDLE) {
        /* Pops the first event from the disk 1 queue. */
        Event job = pop(&disk2_queue);
        disk2_total--;
        /* Creates the disk 2 finish event and pushes it onto the priority queue. */
        rand_time = my_random(DISK2_MIN, DISK2_MAX) + curr_time;
        Event temp = new_event(job.job_num, DISK_FINISH, rand_time);
        p_push(&p_queue, temp, p_total);
        p_total++;
        /* Saves the job number of what is currently in disk 1. */
        disk2 = job.job_num;
        /* Sets the disk 1 state to busy. */
        disk2_state = BUSY;
        disk2_old = curr_time;
      }
    }
  /* If event type is disk finish. */
  } else {
      /* Sends the job from the disk back to the CPU. */
      Event temp = new_event(curr.job_num, ARRIVAL, curr_time);
      p_push(&p_queue, temp, p_total);
      p_total++;

      /* If the job currently in disk 1 matches the current job. */
      if(disk1 == curr.job_num) {
        int time = curr_time - disk1_old;
        disk1_time += time;
        /* Sets the disk 1 state to IDLE. */
        disk1_state = IDLE;
        disk1 = 0;
        disk1_job++;
        if(time > disk1_job_max) {
          disk1_job_max = time;
        }
        fprintf(fp, "At %d Job %d finished at Disk 1.\n", curr.time, curr.job_num);
      /* If the job currently in disk 2 matches the current job. */
      } else if(disk2 == curr.job_num) {
        int time = curr_time - disk2_old;
        disk2_time += time;
        /* Sets the disk 2 state to IDLE. */
        disk2_state = IDLE;
        disk2 = 0;
        disk2_job++;
        if(time > disk2_job_max) {
          disk2_job_max = time;
        }
        fprintf(fp, "At %d Job %d finished at Disk 2.\n", curr.time, curr.job_num);
      }
  }
}

/* Finds the max sizes of the queues. */
void find_max() {
  /* Max for priorty queue. */
  if(p_max < p_total) {
    p_max = p_total;
  }

  /* Max for cpu queue. */
  if(cpu_max < cpu_total) {
    cpu_max = cpu_total;
  }

  /* Max for disk 1 queue. */
  if(disk1_max < disk1_total) {
    disk1_max = disk1_total;
  }

  /* Max for disk 2 queue. */
  if(disk2_max < disk2_total) {
    disk2_max = disk2_total;
  }
}

/* Prints out the statistics. */
void print_stats() {
  /* Average size of each queue. */
  printf("Priority Queue average %f.\n", p_avg/total);
  printf("CPU average %f.\n", cpu_avg/total);
  printf("Disk 1 average %f.\n", disk1_avg/total);
  printf("Disk 2 average %f.\n", disk2_avg/total);

  /* Max size of each queue. */
  printf("Priority Queue max %d.\n", p_max);
  printf("CPU max %d.\n", cpu_max);
  printf("Disk 1 max %d.\n", disk1_max);
  printf("Disk 2 max %d.\n", disk2_max);

  /* Utilization of each component. */
  printf("CPU utilization %f.\n", cpu_time/total_time);
  printf("Disk 1 utilization %f.\n", disk1_time/total_time);
  printf("Disk 2 utilization %f.\n", disk2_time/total_time);

  /* Average response time for each component. */
  printf("CPU average response time %f.\n", cpu_time/cpu_job);
  printf("Disk 1 average response time %f.\n", disk1_time/disk1_job);
  printf("Disk 2 average response time %f.\n", disk2_time/disk2_job);

  /* Max response time for each component. */
  printf("CPU max response time %d.\n", cpu_job_max);
  printf("Disk 1 max response time %d.\n", disk1_job_max);
  printf("Disk 2 max response time %d.\n", disk2_job_max);

  /* The throughput of each component. */
  printf("Throughput of CPU %f.\n", cpu_job/total_time);
  printf("Throughput of Disk 1 %f.\n", disk1_job/total_time);
  printf("Throughput of Disk 2 %f.\n", disk2_job/total_time);
}
