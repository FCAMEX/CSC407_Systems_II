/*-------------------------------------------------------------------------*
 *---									---*
 *---		wordCounterHeader.h					---*
 *---									---*
 *---	    This file includes common header files and declares structs	---*
 *---	needed by .c files of the wordCounter program.			---*
 *---									---*
 *---	----	----	----	----	----	----	----	----	---*
 *---									---*
 *---	Version 1.0		2018 September 9	Joseph Phillips	---*
 *---									---*
 *-------------------------------------------------------------------------*/

//---			    Common inclusions:			---//
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>


//---			    Common constants:			---//
#define		SMALL_TEXT_LEN	256


//---			      Common types:			---//
struct		TreeNode
{
  char*			wordCPtr_;
  int			count_;
  struct TreeNode*	leftPtr_;
  struct TreeNode*	rightPtr_;
};


struct		ListNode
{
  char*			wordCPtr_;
  int			count_;
  struct ListNode*	nextPtr_;
};


//---		    Declarations of global functions:			---//
extern int textLen;

struct TreeNode* buildTree(FILE* filePtr);
void printTree(struct TreeNode* nodePtr);
void freeTree(struct TreeNode* nodePtr);
struct ListNode* buildList(FILE* filePtr);
void printList(struct ListNode* firstPtr);
void freeList(struct ListNode* firstPtr);
