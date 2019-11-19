#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#define MAX_SIZE 4096

enum CType {
	Atom = 0,
	And,
	Or,
	Not
};

struct _CTree {
	enum CType type;
	int n;
	int children_cnt;
	struct _CTree **children;
};

typedef struct _CTree CTree;
typedef CTree *pCTree;

bool isRightForm(char* str);
pCTree asCTree(char* str);
void _printCTree(pCTree ctree);
void printCTree(pCTree ctree);

void DNF(pCTree ctree);
char *printDNF(pCTree ctree);
void NNF(pCTree ctree);

pCTree not(pCTree ctree) ;
void m_and(pCTree tree,pCTree a, pCTree b);
void m_or(pCTree tree,pCTree a, pCTree b);
void m_distr(pCTree tree,pCTree a, pCTree b);
char *replaceAll(char *s, const char *olds, const char *news);
void _toDNF(char* result, pCTree ctree);

char* _toBuffer(pCTree ctree);

bool isSAT(char *str){
	bool isitsat = true;
  int count = 0;

	char copystr[1024];
	strcpy(copystr, str);
  char *ptr = strtok(copystr, " ");      // " " 공백 문자를 기준으로 문자열을 자름, 포인터 반환
  int a[MAX_SIZE] = {0};

  while (ptr != NULL)               // 자른 문자열이 나오지 않을 때까지 반복
  {
      a[count] = atoi(ptr);
      //printf("a[%d] = %d\n", count, a[count]);    //DEBUGING
      ptr = strtok(NULL, " ");      // 다음 문자열을 잘라서 포인터를 반환
      count++;
  }

  for(int i = 0; i < count; i++){
    for(int j = count -1; j > i ; j--){
      if(a[i]*(-1) == a[j]){
        //printf("-a[%d] == a[%d] == %d", i, j, a[i]);    //DEBUGING
				isitsat = false;
      }
    }
  }

	return isitsat;
}

void printSol(char *dnfstr){
  int count = 0;
  char *ptr = strtok(dnfstr, "\n");
	char line[64][1024];

	int count_unsat = 0;
	int satom[MAX_SIZE/2] = {0};
	int untom[MAX_SIZE/2] = {0};
	int sat_i = 0, unsat_i = 0;

	while (ptr != NULL)
	{
			//line[count] = ptr;
			strcpy(line[count], ptr);
			ptr = strtok(NULL, "\n");
			count++;
	}

	for(int i = 0; i < count ; i++){
		bool isit = isSAT(line[i]);
		char *ptr2 = strtok(line[i], " ");
				if(isit) {
					while (ptr2 != NULL)
					{
							satom[sat_i] = atoi(ptr2);
							ptr2 = strtok(NULL, " ");
							sat_i++;
					}
				}
				else {
					count_unsat++;
					while (ptr2 != NULL)
					{
							untom[unsat_i] = atoi(ptr2);
							ptr2 = strtok(NULL, " ");
							unsat_i++;
					}
				}
	}

	if(count_unsat == count) {
		printf("0\nUNSAT\n");
		return;
	}else printf("0\n");

//print satom
	for(int i = 0 ; i < sat_i ; i++){
		if(satom[i] != 0 ) printf("%d ", satom[i]);
		for(int j = sat_i ; j > i ; j--){
			if(abs(satom[i]) == abs(satom [j])) satom[j] = 0;
		}
	}

	//print unsatom
	for(int i = 0 ; i < unsat_i ; i++){
		if(untom[i] != 0 )
			for(int j = 0 ; j < sat_i ; j++){
				if(abs(untom[i]) == abs(satom[j])) untom[i] = 0;
			}
		for(int j = unsat_i ; j > i ; j--){
			if(abs(untom[i]) == abs(untom[j])) untom[j] = 0;
		}
		if(untom[i] != 0) printf("%d ", untom[i]);
	}

	printf("\n");

  return;
}


int main() {

	char str[MAX_SIZE];

	fgets(str,MAX_SIZE,stdin);

	bool option = false;

	if(isRightForm(str)) {
//		printf("It is DNF\n");

		pCTree ctree = asCTree(str);
		if(option) {
			printf("Input: ");
			printf("%s\n",str);
			printf("Tree : ");
			printCTree(ctree);
		}

		NNF(ctree);
		if(option) {
			printf("NNF: ");
			printCTree(ctree);
		}

		DNF(ctree);
		if(option) {
			printf("DNF: ");
			printCTree(ctree);
		}

		char *dnfstr = printDNF(ctree);
		printf("%s", dnfstr);
    printSol(dnfstr);
	}
	else printf("ERROR: Not right propositional logic\n");

	return 0;
}


bool isRightForm(char* str) {
	enum CType type;

	//printf("Right recur: '%s'\n",str);

	if(str[0] == 'a') {
		int n = atoi(str+1);
		type = Atom;
		return n > 0;
	} else {

		char buf[8];
		sscanf(str,"%s",buf);

		if(strcmp("(and",buf) == 0) {
			type = And;
		}
		else if(strcmp("(or",buf) == 0) {
			type = Or;
		}
		else if(strcmp("(not",buf) == 0) {
			type = Not;
		}
		else {
			return false;
		}

		int d = 0;
		for(int i = 0; i < strlen(str); i++) {
			if(str[i] == '(') {
				d++;
			}
			else if(str[i] == ')') {
				d--;
				if(d<0) return false;
			}
		}

		if(d != 0) return false;

		int child_cnt = 0;
		char strbuf[2048];
		int strbuf_c = 0;
		int depth = 1;

		int len = strlen(str);

		switch(type) {

		case And:

			for(int i = 5; i < len; i++) {

				if(str[i] == '(') {
					depth++;
				} else if(str[i] == ')') {
					depth--;
					if(depth == 1) {
						strbuf[strbuf_c] = str[i];
						strbuf[strbuf_c+1] = '\0';
						if(!isRightForm(strbuf)) return false;
						strcpy(strbuf,"");
						strbuf_c = 0;
						child_cnt++;
						continue;

					} else if(depth == 0) {
						if(strlen(strbuf) > 0) {

							strbuf[strbuf_c] = '\0';
							if(!isRightForm(strbuf)) return false;
							strcpy(strbuf,"");
							strbuf_c = 0;
							child_cnt++;

						}
						continue;
					}
				} else if(str[i] == ' ' && depth == 1) {
					if(strlen(strbuf) > 0) {

						strbuf[strbuf_c] = '\0';
						if(!isRightForm(strbuf)) return false;
						strcpy(strbuf,"");
						strbuf_c = 0;
						child_cnt++;


					}
					strcpy(strbuf,"");
					continue;
				}
				strbuf[strbuf_c] = str[i];
				strbuf_c++;
			}

			if(child_cnt < 2)
				return false;

			break;


		case Or:

			for(int i = 4; i < len; i++) {

				if(str[i] == '(') {
					depth++;
				} else if(str[i] == ')') {
					depth--;
					if(depth == 1) {
						strbuf[strbuf_c] = str[i];
						strbuf[strbuf_c+1] = '\0';
						if(!isRightForm(strbuf)) return false;
						strcpy(strbuf,"");
						strbuf_c = 0;
						child_cnt++;
						continue;

					} else if(depth == 0) {
						if(strlen(strbuf) > 0) {

							strbuf[strbuf_c] = '\0';
							if(!isRightForm(strbuf)) return false;
							strcpy(strbuf,"");
							strbuf_c = 0;
							child_cnt++;

						}
						continue;
					}
				} else if(str[i] == ' ' && depth == 1) {
					if(strlen(strbuf) > 0) {

						strbuf[strbuf_c] = '\0';
						if(!isRightForm(strbuf)) return false;
						strcpy(strbuf,"");
						strbuf_c = 0;
						child_cnt++;

					}
					strcpy(strbuf,"");
					continue;
				}
				strbuf[strbuf_c] = str[i];
				strbuf_c++;
			}
			if(child_cnt < 2)
				return false;

			break;

		case Not:

			child_cnt = 1;

			char* pr = str+5;
			char temp[256];
			strcpy(temp,pr);
			temp[strlen(pr)-1] = '\0';

			int d = 0;
			int cnt = 0;

			for(int j = 0; j < strlen(temp); j++) {

				if(temp[j] == '(') {
					if(d==0) cnt++;
					d++;
				}
				else if(temp[j] == 'a' && d == 0) {
					cnt++;
				}
				else if(temp[j] == ')') {
					d--;
				}
			}

			if(cnt > 1) return false;

			return isRightForm(temp);
		}
	}

	return true;
}

pCTree asCTree(char* str) {
	enum CType type;
/*
*/
	//printf("asCTree: '%s'\n",str);

	if(str[0] == 'a') {

		int n = atoi(str+1);
		type = Atom;

		pCTree pc = malloc(sizeof(CTree));
		pc->type = type;
		pc->n = n;
		pc->children_cnt = 0;
		pc->children = malloc(sizeof(CTree*)*2048);

		return pc;
	} else {

		char buf[8];
		sscanf(str,"%s",buf);

		if(strcmp("(and",buf) == 0) {
			type = And;
		}
		else if(strcmp("(or",buf) == 0) {
			type = Or;
		}
		else if(strcmp("(not",buf) == 0) {
			type = Not;
		}
		else {
			return false;
		}


		char strbuf[2048];
		int strbuf_c = 0;
		int depth = 1;

		CTree** children;
		children = malloc(sizeof(CTree*)*1024);

//		char s_children[1024][1024];
		char** s_children= malloc(sizeof(char*)*1024);
		for(int i = 0; i < 1024; i++) s_children[i] = malloc(sizeof(char)*1024);


		int child_cnt = 0;

				int len = strlen(str);


		switch(type) {

		case And:
			for(int i = 5; i < len; i++) {
				if(str[i] == '(') {
					depth++;
				} else if(str[i] == ')') {
					depth--;
					if(depth == 1) {


						strbuf[strbuf_c] = str[i];
						strbuf[strbuf_c+1] = '\0';

						strcpy(s_children[child_cnt],strbuf);
						child_cnt++;

						strcpy(strbuf,"");
						strbuf_c = 0;
						continue;

					} else if(depth == 0) {
						if(strlen(strbuf) > 0) {

							strbuf[strbuf_c] = '\0';

						strcpy(s_children[child_cnt],strbuf);
						child_cnt++;
							strcpy(strbuf,"");
							strbuf_c = 0;

						}
						continue;
					}
				} else if(str[i] == ' ' && depth == 1) {
					if(strlen(strbuf) > 0) {

						strbuf[strbuf_c] = '\0';

						strcpy(s_children[child_cnt],strbuf);
						child_cnt++;

						strcpy(strbuf,"");
						strbuf_c = 0;


					}
					strcpy(strbuf,"");
					continue;
				}
				strbuf[strbuf_c] = str[i];
				strbuf_c++;
			}
			break;

		case Or:

			for(int i = 4; i < len; i++) {
				if(str[i] == '(') {
					depth++;
				} else if(str[i] == ')') {
					depth--;
					if(depth == 1) {



						strbuf[strbuf_c] = str[i];
						strbuf[strbuf_c+1] = '\0';

						strcpy(s_children[child_cnt],strbuf);
						child_cnt++;

						strcpy(strbuf,"");
						strbuf_c = 0;
						continue;

					} else if(depth == 0) {
						if(strlen(strbuf) > 0) {

							strbuf[strbuf_c] = '\0';

						strcpy(s_children[child_cnt],strbuf);
						child_cnt++;

							strcpy(strbuf,"");
							strbuf_c = 0;

						}
						continue;
					}
				} else if(str[i] == ' ' && depth == 1) {
					if(strlen(strbuf) > 0) {

						strbuf[strbuf_c] = '\0';

						strcpy(s_children[child_cnt],strbuf);
						child_cnt++;

						strcpy(strbuf,"");
						strbuf_c = 0;


					}
					strcpy(strbuf,"");
					continue;
				}
				strbuf[strbuf_c] = str[i];
				strbuf_c++;
			}
			break;

		case Not:


			type = Not;

			char* pr = str+5;
			char temp[1024];
			strcpy(temp,pr);
			temp[strlen(pr)-1] = '\0';

			pCTree* pchilds = malloc(sizeof(pCTree*)*1024);

			pCTree pchild = asCTree(temp);

			pchilds[0] = pchild;


			pCTree pc = malloc(sizeof(CTree));

			pc->type = Not;
			pc->n = 0;
			pc->children_cnt = 1;
			pc->children = malloc(sizeof(pCTree));
			pc->children = pchilds;


			return pc;
			// break;
		}


		pCTree pc = malloc(sizeof(CTree));
		pc->type = type;
		pc->n = 0;
		pc->children_cnt = child_cnt;
		pc->children = malloc(sizeof(CTree*)*2048);

		//printf("str: '%s'\n",str);
		//printf("Trr: %s\n",_toBuffer(asCTree(str)));

		for(int i = 0; i < child_cnt; i++) {
		//	printf("%d: parse: '%s', %ld\n",i,s_children[i],s_children[i]);
			pCTree t = asCTree(s_children[i]);
		//	printf("%d: parse!\n",i);
			pc->children[i] = t;

		}

		//printCTree(pc);

		return pc;

	}

	return 0x0;

}

int dep = 0;

char* _toBuffer(pCTree ctree) {
/*
	for(int i = 0; i < dep; i++)
		printf("  ");
	dep++;

	printf("Buffer: ");
	printCTree(ctree);
*/
	char* buf = malloc(sizeof(char)*2048);
	enum CType type = ctree->type;

	if(type == Atom) {
		sprintf(buf,"a%d",ctree->n);
	}

	if(type != Atom) {

		if(ctree->children_cnt < 1) {
			printf("Hello! %d t: %d\n",ctree->n,type);
			exit(1);
		}

		sprintf(buf,"%s", type == And? "(and " : type == Or? "(or ": "(not ");
		for(int i = 0; i < ctree->children_cnt; i++)  {


			sprintf(buf,"%s%s ",buf,_toBuffer(ctree->children[i]));


		}

		int len = strlen(buf);
		buf[len-1] = ')';
		buf[len] = '\0';

	}

	return buf;

}
void printCTree(pCTree ctree) {

	_printCTree(ctree);
	printf("\n");
}
void _printCTree(pCTree ctree) {

	enum CType type = ctree->type;

	if(type == Atom) {
		printf("a%d",ctree->n);
	}

	if(type != Atom) {
		printf("%s", type == And? "(and " : type == Or? "(or ": "(not ");
		for(int i = 0; i < ctree->children_cnt; i++)  {
			_printCTree(ctree->children[i]);
			printf(" ");
		}
		printf("\b)");
	}

}
void DNF(pCTree ctree) {

	pCTree t = asCTree(_toBuffer(ctree));

	ctree->type = t->type;
	ctree->n = t->n;
	ctree->children_cnt = t->children_cnt;
	ctree->children = t->children;




	pCTree* children = malloc(sizeof(pCTree)*1024);

	int children_cnt = ctree->children_cnt;
	if(!(ctree->type == Atom))
	for(int i = 0; i < ctree->children_cnt; i++) {

		char* buf = _toBuffer(ctree->children[i]);

		children[i] = asCTree(buf);

	}

	switch(ctree->type) {

	case Atom:
		return;
	case Not:
		return;

	case Or:

		for(int i = 0; i < children_cnt; i++) {

			DNF(children[i]);

		}

		m_or(ctree,children[0],children[1]);

		for(int i = 2; i < children_cnt; i++) {
			m_or(ctree,ctree,children[i]);
		}
		return;

	case And:

		for(int i = 0; i < children_cnt; i++) {
			DNF(children[i]);
		}

		m_distr(ctree,children[0],children[1]);


		for(int i = 2; i < children_cnt; i++) {
			m_distr(ctree,ctree,children[i]);
		}
		return;
	}
}

void m_and(pCTree tree,pCTree a, pCTree b) {

	pCTree a1 = asCTree(_toBuffer(a));
	pCTree b1 = asCTree(_toBuffer(b));

	pCTree* children = malloc(sizeof(pCTree)*2);
	children[0] = a1;
	children[1] = b1;

	tree->type = And;
	tree->n = 0;
	tree->children_cnt = 2;
	tree->children = children;

}
void m_or(pCTree tree,pCTree a, pCTree b) {
	pCTree a1 = asCTree(_toBuffer(a));
	pCTree b1 = asCTree(_toBuffer(b));

	pCTree* children = malloc(sizeof(pCTree)*2);
	children[0] = a1;
	children[1] = b1;

	tree->type = Or;
	tree->n = 0;
	tree->children_cnt = 2;
	tree->children = children;
}

int dis_dep = 0;

void m_distr(pCTree tree,pCTree a, pCTree b) {

	a = asCTree(_toBuffer(a));
	b = asCTree(_toBuffer(b));

	if(a->type == Or) {
		for(int i = 0; i < a->children_cnt; i++) {
			m_distr(a->children[i],a->children[i],b);
		}

		m_or(tree,a->children[0],a->children[1]);
		for(int i = 2; i < a->children_cnt; i++) {
			m_or(tree,tree,a->children[i]);
		}

	} else if(b->type == Or) {
		for(int i = 0; i < b->children_cnt; i++) {
			m_distr(b->children[i],b->children[i],a);
		}

		m_or(tree,b->children[0],b->children[1]);
		for(int i = 2; i < b->children_cnt; i++) {
			m_or(tree,tree,b->children[i]);
		}
	} else {
		m_and(tree,a,b);
	}
}

void NNF(pCTree ctree) {

	pCTree child;

	switch(ctree->type) {

	case Atom:
		return;

	case And:
	case Or:


		for(int i = 0; i < ctree->children_cnt; i++) {
			NNF(ctree->children[i]);
		}
		return;

	case Not:

		switch((ctree->children[0])->type) {

		case Atom:
			return;


	 case Not:

		child = (ctree->children[0])->children[0];

		ctree->type = child->type;
		ctree->n = child->n;
		ctree->children_cnt = child->children_cnt;
		ctree->children = malloc(sizeof(pCTree)*child->children_cnt);

		for(int i = 0; i < child->children_cnt; i++) {
			ctree->children[i] = child->children[i];
		}


			NNF(ctree);

			return;

		case And:
			ctree->type = Or; // inverse

			pCTree* children = (ctree->children[0])->children;
			int children_cnt = (ctree->children[0])->children_cnt;

			ctree->children_cnt = children_cnt;
			ctree->children = malloc(sizeof(pCTree)*children_cnt);

			for(int i = 0; i < children_cnt; i++) {

				pCTree new_ctree;
				new_ctree = not(children[i]);

				(ctree->children)[i] = new_ctree;

			}

			NNF(ctree);

			break;

		case Or:
			ctree->type = And; // inverse

			children = (ctree->children[0])->children;
			children_cnt = (ctree->children[0])->children_cnt;

			ctree->children_cnt = children_cnt;
			ctree->children = malloc(sizeof(pCTree)*children_cnt);

			for(int i = 0; i < children_cnt; i++) {

				pCTree new_ctree;
				new_ctree = not(children[i]);

				(ctree->children)[i] = new_ctree;

			}

			NNF(ctree);

			break;

		}
	}

}
pCTree not(pCTree ctree) {

	pCTree new_ctree = malloc(sizeof(CTree));

	new_ctree->type = Not;
	new_ctree->n = 0;
	new_ctree->children_cnt = 1;
	new_ctree->children = malloc(sizeof(pCTree));

	new_ctree->children[0] = ctree;

	return new_ctree;
}

char *printDNF(pCTree ctree) {
//void printDNF
	char* str = malloc(sizeof(char)*2048);

	_toDNF(str,ctree);
	int len = strlen(str);

	str[len] = '\n';
	str[len+1] = '\0';

	char* str2 = replaceAll(str,"\n\n","\n");

	while(strlen(str) != strlen(str2)) {
		str = str2;
		str2 = replaceAll(str2,"\n\n","\n");
	}

  return str;
}

void _toDNF(char* result, pCTree ctree) {
	char buf[2048] = "";

	switch(ctree->type) {

	case Atom:
		sprintf(buf,"%d",ctree->n);
		strcpy(result,buf);
		return;

	case Not:
		sprintf(buf,"-%d",ctree->children[0]->n);
		strcpy(result,buf);
		return;

	case And:
		for(int i = 0; i < ctree->children_cnt; i++) {
			char* temp = malloc(sizeof(char)*2048);
			_toDNF(temp,ctree->children[i]);
			sprintf(buf,"%s%s ",buf,temp);
		}
		buf[strlen(buf)-1] = '\0';
		strcpy(result,buf);
		return;

	case Or:

		for(int i = 0; i < ctree->children_cnt; i++) {
			char* temp = malloc(sizeof(char)*2048);
			_toDNF(temp,ctree->children[i]);
			sprintf(buf,"%s%s\n",buf,temp);
		}
		strcpy(result,buf);
		return;
	}
}


char *replaceAll(char *s, const char *olds, const char *news) {
  char *result, *sr;
  size_t i, count = 0;
  size_t oldlen = strlen(olds); if (oldlen < 1) return s;
  size_t newlen = strlen(news);


  if (newlen != oldlen) {
    for (i = 0; s[i] != '\0';) {
      if (memcmp(&s[i], olds, oldlen) == 0) count++, i += oldlen;
      else i++;
    }
  } else i = strlen(s);


  result = (char *) malloc(i + 1 + count * (newlen - oldlen));
  if (result == NULL) return NULL;


  sr = result;
  while (*s) {
    if (memcmp(s, olds, oldlen) == 0) {
      memcpy(sr, news, newlen);
      sr += newlen;
      s  += oldlen;
    } else *sr++ = *s++;
  }
  *sr = '\0';

  return result;
}
