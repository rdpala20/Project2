 /* Project 2 - Multithreaded calculator */
// Name: Rodrigo Palacios


#include "calc.h"

pthread_t adderThread;
pthread_t degrouperThread;
pthread_t multiplierThread;
pthread_t readerThread;
pthread_t sentinelThread;

char buffer[BUF_SIZE]; // buffer size
int num_ops; // total number of operations done

/* Step 3: add mutual exclusion */
//initialize the mutual exclusion lock
pthread_mutex_t mutexLock = PTHREAD_MUTEX_INITIALIZER;


/* Step 6: add condition flag varaibles */
int flag_add; //flag for adder
int flag_mult; // flag for multiplier
int flag_degroup; // flag for degrouper


/* Step 7: use a semaphore */
static sem_t semLock; // intialize semaphore


/* Utiltity functions provided for your convenience */

/* int2string converts an integer into a string and writes it in the
   passed char array s, which should be of reasonable size (e.g., 20
   characters).  */
char *int2string(int i, char *s)
{
    sprintf(s, "%d", i);
    return s;
}

/* string2int just calls atoi() */
int string2int(const char *s)
{
    return atoi(s);
}

/* isNumeric just calls isdigit() */
int isNumeric(char c)
{
    return isdigit(c);
}

/* End utility functions */


void printErrorAndExit(char *msg)
{
    msg = msg ? msg : "An unspecified error occured!";
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

int timeToFinish()
{
    /* be careful: timeToFinish() also accesses buffer */
    return buffer[0] == '.';
}

/* Looks for an addition symbol "+" surrounded by two numbers, e.g. "5+6"
   and, if found, adds the two numbers and replaces the addition subexpression
   with the result ("(5+6)*8" becomes "(11)*8")--remember, you don't have
   to worry about associativity! */
void *adder(void *arg)
{
    int bufferlen; // length of buffer
    int value1, value2; // values to be added
    int startOffset, remainderOffset; // beginning to end of buffer
    int i; // variable
    int result; //v1+v2
    char temp[50]; //temporary buffer to hold result

    while (1) {

		/* Step 3: add mutual exclusion */
	startOffset = remainderOffset = -1;
	value1 = value2 = -1;

  // start mutal exclusion lock, if its the end unlock
  pthread_mutex_lock(&mutexLock);
	if (timeToFinish()) {
    pthread_mutex_unlock(&mutexLock);
	    return NULL;
	}

	/* storing this prevents having to recalculate it in the loop */
	bufferlen = strlen(buffer);
	/* Step 2: implement adder */
  for (i = 0; i < bufferlen; i++) {
    //check for sentinel character
    if(buffer[i]==';')
      break;
    // if number in buffer at i
  if(isdigit(buffer[i])){
    // looks for '+' next to a number
    if(buffer[i+1] == '+' && isdigit(buffer[i+2])){
       startOffset = i; //initalize startoffset to beginning
       value1 = string2int(buffer+i); //convert the string values to integers
       value2 = string2int(buffer+2);
       result = value1 + value2; // add integers to result
       i++; //increment counter
       //do while loop to get to the end of the buffer until sentinel
    do {
      i++;
      if(buffer[i] == ';')
        break;
    } while ((buffer[i]));
     // end of buffer before sentinel is the remainderOffset
      remainderOffset = i;
      // conert the integer to a string in a temporary char array
      sprintf(temp,"%d", result);
      //copy the temporary array(Result in string) into the current buffer at the starting point
      strcpy(buffer + startOffset, temp);
      // copy the rest of the characters in the buffer into the current one
      strcpy(buffer + startOffset + strlen(temp), buffer + remainderOffset);
      //increment operations being performed
      num_ops++;
      //because we added, sum turns to 1
      }
    }
}

	// something missing?
	/* Step 3: free the lock */
  pthread_mutex_unlock(&mutexLock);

	/* Step 6: check progress */
  sem_wait(&semLock);
  sem_post(&semLock);
	/* Step 5: let others play */
  sched_yield();
    }
}

/* Looks for a multiplication symbol "*" surrounded by two numbers, e.g.
   "5*6" and, if found, multiplies the two numbers and replaces the
   mulitplication subexpression with the result ("1+(5*6)+8" becomes
   "1+(30)+8"). */
void *multiplier(void *arg)
{
    int bufferlen;
    int value1, value2;
    int startOffset, remainderOffset;
    int i;
    int result;
    char temp[50];


    while (1) {
		/* Step 3: add mutual exclusion */


	startOffset = remainderOffset = -1;
	value1 = value2 = -1;

  pthread_mutex_lock(&mutexLock);
	if (timeToFinish()) {
      pthread_mutex_unlock(&mutexLock);
	    return NULL;
	}

	/* storing this prevents having to recalculate it in the loop */
	bufferlen = strlen(buffer);

	/* Step 2: implement multiplier */
  for (i = 0; i < bufferlen; i++) {
    if(buffer[i]==';')
      break;
  if(isdigit(buffer[i])){
    if(buffer[i+1] == '*' && isdigit(buffer[i+2])){
       startOffset = i;
       value1 = atoi(buffer+i);
       value2 = atoi(buffer+i+2);
       result = value1 * value2;
       i++;
    do {
      i++;
      if(buffer[i] == ';')
        break;
    } while ((buffer[i]));
      remainderOffset = i;
      sprintf(temp,"%d", result);
      strcpy(buffer + startOffset, temp);
      strcpy(buffer + startOffset + strlen(temp), buffer + remainderOffset);
      num_ops++;
      }
    }
}

	// something missing?
	/* Step 3: free the lock */
  pthread_mutex_unlock(&mutexLock);

	/* Step 6: check progress */
  sem_wait(&semLock);
  sem_post(&semLock);
	/* Step 5: let others play */
  sched_yield();
    }
}


/* Looks for a number immediately surrounded by parentheses [e.g.
   "(56)"] in the buffer and, if found, removes the parentheses leaving
   only the surrounded number. */
void *degrouper(void *arg)
{
    int bufferlen;
    int i;
    int j;
    int count = 0;
    int num;

    while (1) {
		/* Step 3: add mutual exclusion */
    flag_degroup =1;
  pthread_mutex_lock(&mutexLock);
	if (timeToFinish()) {
      pthread_mutex_unlock(&mutexLock);
	    return NULL;
	}

	/* storing this prevents having to recalculate it in the loop */
	bufferlen = strlen(buffer);
  num =1;
	/*Step 2: implement degrouper*/
	for (i = 0; i < bufferlen; i++) { // loop through the buffer length
      if(buffer[i]==';')
        break;
      if((buffer[i+2] == '+' || buffer[i+2] == '*') && buffer[i] == '(')// check for (#+#)
        break; //operation still needed to be done
      if(buffer[i] == '+' || buffer[i] == '*')
        break;
      if(buffer[i] == '(' || buffer[i] == ')'){ // check for begining ( or end )
         buffer[i] = ' '; // replace parantheses to empty spaces
         num = 0;
       }
    }
  // loop through buffer replacing the current place of where there was a
  if(num == 0){
  for(j = 0; buffer[j]; j++){
      if (buffer[j] != ' ') // if there is a space we move the array
         buffer[count++] = buffer[j];
  }
  //include null at the end
  buffer[count] = '\0';
  num_ops++;
}

	// something missing?
	/* Step 3: free the lock */
  pthread_mutex_unlock(&mutexLock);

	/* Step 6: check progress */
  sem_wait(&semLock);

  sem_post(&semLock);

	/* Step 5: let others play */
  sched_yield();
    }
}


/* sentinel waits for a number followed by a ; (e.g. "453;") to appear
   at the beginning of the buffer, indicating that the current
   expression has been fully reduced by the other threads and can now be
   output.  It then "dequeues" that expression (and trailing ;) so work can
   proceed on the next (if available). */
void *sentinel(void *arg)
{
    char numberBuffer[20];
    int bufferlen;
    int i;


    while (1) {

		/* Step 3: add mutual exclusion */
  pthread_mutex_lock(&mutexLock);
	if (timeToFinish()) {
      pthread_mutex_unlock(&mutexLock);
	    return NULL;
	}




	/* storing this prevents having to recalculate it in the loop */
	bufferlen = strlen(buffer);

	for (i = 0; i < bufferlen; i++) {
	    if (buffer[i] == ';') {
		if (i == 0) {
		    printErrorAndExit("Sentinel found empty expression!");
		} else {
		    /* null terminate the string */
		    numberBuffer[i] = '\0';
		    /* print out the number we've found */
		    fprintf(stdout, "%s\n", numberBuffer);
		    /* shift the remainder of the string to the left */
		    strcpy(buffer, &buffer[i + 1]);
		    break;
		}
	    } else if (!isNumeric(buffer[i])) {
		break;
	    } else {
		numberBuffer[i] = buffer[i];
	    }
	}

	// something missing?
	/* Step 6: check for progress */
  sem_wait(&semLock);

  sem_post(&semLock);
	/* Step 5: let others play, too */
  pthread_mutex_unlock(&mutexLock);
  sched_yield();
    }
}

/* reader reads in lines of input from stdin and writes them to the
   buffer */
void *reader(void *arg)
{
    while (1) {
	char tBuffer[100];
	int currentlen;
	int newlen;
	int free;

	fgets(tBuffer, sizeof(tBuffer), stdin);

	/* Sychronization bugs in remainder of function need to be fixed */

	newlen = strlen(tBuffer);
	currentlen = strlen(buffer);

	/* if tBuffer comes back with a newline from fgets, remove it */
	if (tBuffer[newlen - 1] == '\n') {
	    /* shift null terminator left */
	    tBuffer[newlen - 1] = tBuffer[newlen];
	    newlen--;
	}

	/* -1 for null terminator, -1 for ; separator */
	free = sizeof(buffer) - currentlen - 2;

	while (free < newlen) {


	}

	/* Step 3: add mutual exclusion */
  pthread_mutex_lock(&mutexLock);
	/* we can add another expression now */
  sem_wait(&semLock);
	strcat(buffer, tBuffer);
	strcat(buffer, ";");
  sem_post(&semLock);
  pthread_mutex_unlock(&mutexLock);
  sched_yield();
	/* Step 6: reset flag variables indicating progress */

	/* Stop when user enters '.' */
	if (tBuffer[0] == '.') {
	    return NULL;
	}
    }
}


/* Where it all begins */
int smp3_main(int argc, char **argv)
{
    void *arg = 0;		/* dummy value */

	/* Step 7: use a semaphore */
  sem_init(&semLock,0,1);
    /* let's create our threads */
    if (pthread_create(&multiplierThread, NULL, multiplier, arg)
	|| pthread_create(&adderThread, NULL, adder, arg)
	|| pthread_create(&degrouperThread, NULL, degrouper, arg)
	|| pthread_create(&sentinelThread, NULL, sentinel, arg)
	|| pthread_create(&readerThread, NULL, reader, arg)) {
	printErrorAndExit("Failed trying to create threads");
    }
    pthread_join(sentinelThread, NULL);
    /* you need to join one of these threads... but which one? */
    pthread_detach(multiplierThread);
    pthread_detach(adderThread);
    pthread_detach(degrouperThread);
    pthread_detach(sentinelThread);
    pthread_detach(readerThread);
	/* Step 1: we have to join on the ________ thread. */

    /* everything is finished, print out the number of operations performed */
    fprintf(stdout, "Performed a total of %d operations\n", num_ops);

	// TODO destroy semaphores and mutex
    pthread_mutex_destroy(&mutexLock);
    sem_destroy(&semLock);
    return EXIT_SUCCESS;
}
