/*-------------------------------------------------------------------------*
 *---																	---*
 *---		student.c													---*
 *---	----	----	----	----	----	----	----	----	---*
 *-------------------------------------------------------------------------*/
 
 
 #include	"sleepyProfHeaders.h"

 int		isStillClassTime	= 1;
 
 pid_t		deanPid;
 
 int 		numberOfSnores = 0;
 
 int 		numberOfComplaints = 0;

 
 void classDismissedHandler(int sigNum){
	 
	 isStillClassTime = 0;
	 printf("Student \"Time for lunch!\"\n");
	 
 }
 
 void profTeachHandler(int sigNum){
	 
	 printf("(Student scribbles notes.)\n");
 }
 
 void profSnoreHandler(int sigNum){
	 numberOfSnores++;
	 if(numberOfSnores < NUM_SNORES_BEFORE_STUDENT_COMPLAINS ){
		 printf("Student \"%s?\"\n",PROFS_NAME);
	 }else{
		 numberOfSnores = 0;
		 numberOfComplaints++;
		 
		 if(numberOfComplaints < NUM_COMPLAINTS_TO_PROF_BEFORE_COMPLAIN_TO_DEAN){
			kill(getppid(), COMPLAIN_SIGNAL);
			printf("Student \"%s!\"\n",PROFS_NAME);
		 } else if(numberOfComplaints >= NUM_COMPLAINTS_TO_PROF_BEFORE_COMPLAIN_TO_DEAN){
			kill(deanPid, COMPLAIN_SIGNAL);
			numberOfComplaints = 0;
			printf("Student \"I have had enough!  I'm calling the Dean.\"\n");
		 }
		 		 
	 }
	 
 }
 
 void installHandlers()
 {
	struct sigaction pt;
	struct sigaction cd;
	struct sigaction ps;
	memset(&pt, '\0', sizeof(struct sigaction));
	memset(&cd, '\0', sizeof(struct sigaction));
	memset(&ps, '\0', sizeof(struct sigaction));
	
	pt.sa_handler = profTeachHandler;
	cd.sa_handler = classDismissedHandler;
	ps.sa_handler = profSnoreHandler;
	
	sigaction(PROF_TEACH_SIGNAL, &pt, NULL);
	sigaction(CLASS_DISMISSED_SIGNAL, &cd, NULL);
	sigaction(PROF_SNORE_SIGNAL, &ps, NULL);
	 
 }
 
 
 int main (int argc,  char*		argv[]){
	 
	 
	 if( (argc < 2) || (argv[1] == NULL) || (argv[1] == '\0')){
		printf("Student \"Hey!  You did not tell me the Dean's number so I can complain if I need to\"\n");
		exit(EXIT_FAILURE);
	 }
	 
	 deanPid = atol(argv[1]);
		 
	 installHandlers();
	 
	 while  (isStillClassTime)
	 {
		sleep(1);
	 }

	return(EXIT_SUCCESS);
 }