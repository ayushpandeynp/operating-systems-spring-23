#ifndef RECORD
#define RECORD

#define SIZEofBUFF 20
#define NumOfCourses 8

typedef struct Record Record;

struct Record
{
    long studentID;
    char firstname[SIZEofBUFF];
    char lastname[SIZEofBUFF];
    float marks[NumOfCourses];
    float gpa;
};

#endif