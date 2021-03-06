/*-------------------------------------------------------------------------*
 *---																	---*
 *---		prof.c														---*
 *---																	---*
 *---	----	----	----	----	----	----	----	----	---*
 *---																	---*
 *---	Version 1a					Fernando Araujo						---*
 *---																	---*
 *-------------------------------------------------------------------------*/

#include	"sleepyProfHeaders.h"


//  PURPOSE:  To hold '1' while class still is in session, or '0' after class
//	is over.
int		isStillClassTime	= 1;


//  PURPOSE:  To hold '2' when the prof is quite awake, '1' when the prof is
//	about to fall asleep, or '0' when the prof is asleep.
int		awakeLevel		= 2;


//  PURPOSE:  To hold the process id number of the student (child) process.
pid_t		studentPid;


//  PURPOSE:  To tell how many facts are known for each subject.
#define		NUM_FACTS_PER_SUBJECT	4


//  PURPOSE:  To tell the physics facts.
const char*	PHYSICS_KNOWLEDGE[NUM_FACTS_PER_SUBJECT]	=
		{ "F = m*a is a special case of F = dp/dt = m*dv/dt + v*dm/dt"
		  " when dm/dt = 0.",

		  "Fermions have 1/2 spin and cannot occupy the same quantum"
		  " state, Bosons have integer spin and can occupy the same"
		  " quantum state.",

		  "The electron-electron repulsion between atoms and molecules"
		  " supports the structure of most matter at our scale. If "
		  "gravity overcomes this electron repulsion, then electrons "
		  "collapse into the nucleus and matter becomes a neutron "
		  "star.",

		  "There is a large Black hole at the center of our galaxy."
		};

//  PURPOSE:  To tell the chemistry facts.
const char*	CHEMISTRY_KNOWLEDGE[NUM_FACTS_PER_SUBJECT]	=
		{"In SN2 reactions, the nucleophile puts electron density into"
		 " the anti-bonding orbital of leaving group, thus weakening"
		 " the bond between the leaving group and the substrate.",

		 "The transition state of a reaction is the configuration of "
		 "highest energy.",

		 "The energy difference between the reactants and the "
		 "transition state determines the rate of reaction",

		 "The Diels-Alder reaction is a popular way to make cyclic "
		 "compounds with atoms other than carbon."
		};


//  PURPOSE:  To tell the biology facts.
const char*	BIOLOGY_KNOWLEDGE[NUM_FACTS_PER_SUBJECT]	=
		{"Allopatric speciation happens when some geological barrier "
		 "forms in the range of a species.  The barrier prevents "
		 "genetic interchange and the previous single species can turn"
		 " into two or more by genetic drift and different selective"
		 " pressures.",

		 "A classic case of Allopatric speciation is with chimpanzees"
		 " north of the Congo river and Bonobos south of it.",

		 "The Hox genes control body plan of an embryo from head to "
		 "tail in creatures as diverse from earthworms to fruit flies "
		 "to humans.  That implies we all had a common ancestor "
		 "hundreds of millions of years ago.",

		 "The Krebs cycle is very important because it is how all "
		 "known aerobic organisms turn carbohydrates, fats and protein"
		 " into energy."
		};


//  PURPOSE:  To make the professor wake up after receiving the COMPLAIN_SIGNAL.
//	If the signal comes from the dean then sets 'awakeLevel = 2' and
//	prints:
//		"Prof \"(Oops! The Dean caught me sleeping on the job!)\"\n"
//		"Prof \"Now as I was saying . . .\"\n"
//
// 	If the signal *merely* comes from the student then sets 'awakeLevel = 1'
//	and prints:
//		"Prof \"Huh?  What?  Oops!  I must have fallen asleep!\"\n"
//		"Prof \"Now as I was saying . . .\"\n".
void		wakeUpHandler		(int		sigNum,
					 siginfo_t*	infoPtr,
					 void*		vPtr
					)
{
  // YOUR CODE HERE
   if(infoPtr->si_pid == getppid() ) //if dean signal
  {
	  awakeLevel = 2;
	  printf("Prof \"(Oops! The Dean caught me sleeping on the job!)\"\n");
	  printf("Prof \"Now as I was saying . . .\"\n");
  }else if(infoPtr->si_pid == studentPid){ //if student signal
	
	  awakeLevel = 1;
	  printf("Prof \"Huh?  What?  Oops!  I must have fallen asleep!\"\n");
	  printf("Prof \"Now as I was saying . . .\"\n");
  }
}


//  PURPOSE:  To make this process stop by setting 'isStillClassTime = 0',
//	and to tell the student process to stop by sending it
//	CLASS_DISMISSED_SIGNAL.  Also prints:
//		"Prof \"Class dismissed!\"\n"
void		classDismissedHandler
				(int		sigNum
				)
{
  // YOUR CODE HERE
  isStillClassTime = 0;
  kill(studentPid,CLASS_DISMISSED_SIGNAL);
  printf("Prof \"Class dismissed!\"\n");
}


//  PURPOSE:  To install 'wakeUpHandler()' for 'COMPLAIN_SIGNAL' and
//	'classDismissedHandler()' for 'CLASS_DISMISSED_SIGNAL'.
void		installHandlers	()
{
	// YOUR CODE HERE
	struct sigaction sa;
	struct sigaction cd;
	memset(&sa, '\0', sizeof(struct sigaction));
	memset(&cd, '\0', sizeof(struct sigaction));
	
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = wakeUpHandler;
	cd.sa_handler = classDismissedHandler;
	
	sigaction(COMPLAIN_SIGNAL, &sa, NULL);
	sigaction(CLASS_DISMISSED_SIGNAL, &cd, NULL);
}


//  PURPOSE:  To print the lesson, and to send 'PROF_TEACH_SIGNAL' to the
//	student.
void		teach		(pid_t		studentPid,
				 subject_ty	subject
				)
{
  const char**	cPtrPtr;

  switch  (subject)
  {
  case PHYSICS_SUBJECT :
    cPtrPtr	= PHYSICS_KNOWLEDGE;
    break;

  case CHEMISTRY_SUBJECT :
    cPtrPtr	= CHEMISTRY_KNOWLEDGE;
    break;

  case BIOLOGY_SUBJECT :
    cPtrPtr	= BIOLOGY_KNOWLEDGE;
    break;  
  }

  printf("Prof \"%s\"\n",cPtrPtr[rand() % NUM_FACTS_PER_SUBJECT]);
  // YOUR CODE HERE
  kill(studentPid, PROF_TEACH_SIGNAL);
}


//  PURPOSE:  To send the 'PROF_SNORE_SIGNAL' to the student.
void		snore		(pid_t		studentPid
				)
{
  // YOUR CODE HERE
  printf("Prof (\"Snore!\")\n");
  kill(studentPid, PROF_SNORE_SIGNAL);
}


//  PURPOSE:  To start the student (child) process.  The child runs
//	'STUDENT_PROGRAM' with the command line argument 'text' (telling the
//	 process id of the dean).  It prints
//	 	 "Student \"I cannot find my classroom!\"\n"
//	and does:
//		exit(EXIT_FAILURE);
//	The parent process returns the process id of the student child process.
pid_t		obtainStudent	()
{
  char	text[MAX_LINE];
  pid_t	childPid;

  snprintf(text,MAX_LINE,"%d",getppid());

  // YOUR CODE HERE
  childPid = fork();
  
  if(childPid == 0)
  {
	  execl(STUDENT_PROGRAM, STUDENT_PROGRAM, text, NULL);
	  printf("Student \"I cannot find my classroom!\"\n");
	  exit(EXIT_FAILURE);
  }

  return(childPid);
}


//  PURPOSE:  To do the work of the professor.  Returns 'EXIT_SUCCESS'.
int		main		(int		argc,
				 char*		argv[]
				)
{
  subject_ty		subject;
  struct sigaction	act;

  srand(getpid());
  installHandlers();

  if  ( (argc < 2)					||
	( (subject = getSubjectFromName(argv[1])) == NO_SUBJECT)
      )
  {
    printf("Prof \"I don't know which course I'm teaching, I quit!\"\n");
    exit(EXIT_FAILURE);
  }

  studentPid	= obtainStudent();

  while  (isStillClassTime)
  {
    if  (awakeLevel > 0)
    {
      teach(studentPid,subject);

      if  ( ((rand() % 1024) / 1024.0) < PROF_FALL_ASLEEP_PROB )
      {
        awakeLevel--;
      }

    }
    else
    {
      snore(studentPid);
    }

    sleep(1);
  }

  return(EXIT_SUCCESS);
}     