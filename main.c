#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <time.h>

//<!-------STRUCT TREE DATA------->
struct tree {
    char name [255];
  int value, height, bf, score;
  struct tree *left, *right;

};
// Print Start
//printing tree in ascii

typedef struct asciinode_struct asciinode;
struct asciinode_struct
{
  asciinode * left, * right;

  //length of the edge from this node to its children
  int edge_length;

  int height;

  int lablen;

  //-1=I am left, 0=I am root, 1=right
  int parent_dir;

  //max supported unit32 in dec, 10 digits max
  char label[11];
};


#define MAX_HEIGHT 1000
int lprofile[MAX_HEIGHT];
int rprofile[MAX_HEIGHT];
#define INFINITY (1<<20)

//adjust gap between left and right nodes
int gap = 3;

//used for printing next node in the same level,
//this is the x coordinate of the next char printed
int print_next;

int MIN (int X, int Y)
{
  return ((X) < (Y)) ? (X) : (Y);
}

int MAX (int X, int Y)
{
  return ((X) > (Y)) ? (X) : (Y);
}

asciinode * build_ascii_tree_recursive(struct tree* t )
{
  asciinode * node;

  if (t == NULL) return NULL;

  node = malloc(sizeof(asciinode));
  node->left = build_ascii_tree_recursive(t->left);
  node->right = build_ascii_tree_recursive(t->right);

  if (node->left != NULL)
  {
    node->left->parent_dir = -1;
  }

  if (node->right != NULL)
  {
    node->right->parent_dir = 1;
  }

  sprintf(node->label, "%d", t->value);
  node->lablen = strlen(node->label);

  return node;
}

//Copy the tree into the ascii node structre
asciinode * build_ascii_tree(struct Node * t)
{
  asciinode *node;
  if (t == NULL) return NULL;
  node = build_ascii_tree_recursive(t);
  node->parent_dir = 0;
  return node;
}

//Free all the nodes of the given tree
void free_ascii_tree(asciinode *node)
{
  if (node == NULL) return;
  free_ascii_tree(node->left);
  free_ascii_tree(node->right);
  free(node);
}

//The following function fills in the lprofile array for the given tree.
//It assumes that the center of the label of the root of this tree
//is located at a position (x,y).  It assumes that the edge_length
//fields have been computed for this tree.
void compute_lprofile(asciinode *node, int x, int y)
{
  int i, isleft;
  if (node == NULL) return;
  isleft = (node->parent_dir == -1);
  lprofile[y] = MIN(lprofile[y], x-((node->lablen-isleft)/2));
  if (node->left != NULL)
  {
	  for (i=1; i <= node->edge_length && y+i < MAX_HEIGHT; i++)
    {
	    lprofile[y+i] = MIN(lprofile[y+i], x-i);
    }
  }
  compute_lprofile(node->left, x-node->edge_length-1, y+node->edge_length+1);
  compute_lprofile(node->right, x+node->edge_length+1, y+node->edge_length+1);
}

void compute_rprofile(asciinode *node, int x, int y)
{
  int i, notleft;
  if (node == NULL) return;
  notleft = (node->parent_dir != -1);
  rprofile[y] = MAX(rprofile[y], x+((node->lablen-notleft)/2));
  if (node->right != NULL)
  {
	  for (i=1; i <= node->edge_length && y+i < MAX_HEIGHT; i++)
    {
	    rprofile[y+i] = MAX(rprofile[y+i], x+i);
    }
  }
  compute_rprofile(node->left, x-node->edge_length-1, y+node->edge_length+1);
  compute_rprofile(node->right, x+node->edge_length+1, y+node->edge_length+1);
}

//This function fills in the edge_length and
//height fields of the specified tree
void compute_edge_lengths(asciinode *node)
{
  int h, hmin, i, delta;
  if (node == NULL) return;
  compute_edge_lengths(node->left);
  compute_edge_lengths(node->right);

  /* first fill in the edge_length of node */
  if (node->right == NULL && node->left == NULL)
  {
	  node->edge_length = 0;
  }
  else
  {
    if (node->left != NULL)
    {
	    for (i=0; i<node->left->height && i < MAX_HEIGHT; i++)
      {
		    rprofile[i] = -INFINITY;
	    }
	    compute_rprofile(node->left, 0, 0);
	    hmin = node->left->height;
    }
    else
    {
	    hmin = 0;
    }
	  if (node->right != NULL)
    {
	    for (i=0; i<node->right->height && i < MAX_HEIGHT; i++)
      {
		    lprofile[i] = INFINITY;
	    }
	    compute_lprofile(node->right, 0, 0);
	    hmin = MIN(node->right->height, hmin);
    }
    else
    {
	    hmin = 0;
    }
	  delta = 4;
	  for (i=0; i<hmin; i++)
    {
	    delta = MAX(delta, gap + 1 + rprofile[i] - lprofile[i]);
    }

    //If the node has two children of height 1, then we allow the
    //two leaves to be within 1, instead of 2
	  if (((node->left != NULL && node->left->height == 1) ||
	      (node->right != NULL && node->right->height == 1))&&delta>4)
    {
      delta--;
    }

    node->edge_length = ((delta+1)/2) - 1;
  }

  //now fill in the height of node
  h = 1;
  if (node->left != NULL)
  {
	  h = MAX(node->left->height + node->edge_length + 1, h);
  }
  if (node->right != NULL)
  {
	  h = MAX(node->right->height + node->edge_length + 1, h);
  }
  node->height = h;
}

//This function prints the given level of the given tree, assuming
//that the node has the given x cordinate.
void print_level(asciinode *node, int x, int level)
{
  int i, isleft;
  if (node == NULL) return;
  isleft = (node->parent_dir == -1);
  if (level == 0)
  {
	  for (i=0; i<(x-print_next-((node->lablen-isleft)/2)); i++)
    {
	    printf(" ");
    }
	  print_next += i;
	  printf("%s", node->label);
	  print_next += node->lablen;
  }
  else if (node->edge_length >= level)
  {
	  if (node->left != NULL)
    {
	    for (i=0; i<(x-print_next-(level)); i++)
      {
		    printf(" ");
	    }
	    print_next += i;
	    printf("/");
	    print_next++;
    }
	  if (node->right != NULL)
    {
	    for (i=0; i<(x-print_next+(level)); i++)
      {
		    printf(" ");
	    }
	    print_next += i;
	    printf("\\");
	    print_next++;
    }
  }
  else
  {
	  print_level(node->left,
                x-node->edge_length-1,
                level-node->edge_length-1);
	  print_level(node->right,
                x+node->edge_length+1,
                level-node->edge_length-1);
  }
}

//prints ascii tree for given Tree structure
void print_ascii_tree(struct tree* t)
{
  asciinode *proot;
  int xmin, i;
  if (t == NULL) return;
  proot = build_ascii_tree(t);
  compute_edge_lengths(proot);
  for (i=0; i<proot->height && i < MAX_HEIGHT; i++)
  {
	  lprofile[i] = INFINITY;
  }
  compute_lprofile(proot, 0, 0);
  xmin = 0;
  for (i = 0; i < proot->height && i < MAX_HEIGHT; i++)
  {
	  xmin = MIN(xmin, lprofile[i]);
  }
  for (i = 0; i < proot->height; i++)
  {
	  print_next = 0;
	  print_level(proot, -xmin, i);
	  printf("\n");
  }
  if (proot->height >= MAX_HEIGHT)
  {
	  printf("(This tree is taller than %d, and may be drawn incorrectly.)\n", MAX_HEIGHT);
  }
  free_ascii_tree(proot);
}
// Print End

int max(int a, int b) { // maximum between 2 numbers
  return (a > b) ? a : b;
}

int getHeight(struct tree *root) { // height of a node
  return (root) ? root->height : 0;
}

int getBalanceFactor(struct tree *root) {  // height of left child - height of right child
  return (root) ? getHeight(root->left) - getHeight(root->right) : 0;
}

int computeNewHeight(struct tree *root) { // new height after insertion or deletion
  return max(getHeight(root->left), getHeight(root->right)) + 1;
}

struct tree *createNode(int value, const char *name, int score) {
  struct tree *newNode = (struct tree*)malloc(sizeof(struct tree));
  strcpy(newNode->name,name);
  newNode->value = value;
  newNode->score = score;
  newNode->height = 1;
  newNode->bf = 0;
  newNode->left = newNode->right = NULL;
  return newNode;
}

struct tree *updateNode(struct tree *root) {
  root->height = computeNewHeight(root);
  root->bf = getBalanceFactor(root);
  return root;
}

// Rotation: https://drive.google.com/drive/folders/1_dRl8kpv5TSJfLvUaGAuoyij0ZreyLSu?usp=sharing
struct tree *leftRotate(struct tree *root) {
  struct tree *pivot = root->right;
  struct tree *temp = pivot->left;
  pivot->left = root;
  root->right = temp;
  root = updateNode(root);
  pivot = updateNode(pivot);
  return pivot;
}

struct tree *rightRotate(struct tree *root) {
  struct tree *pivot = root->left;
  struct tree *temp = pivot->right;
  pivot->right = root;
  root->left = temp;
  root = updateNode(root);
  pivot = updateNode(pivot);
  return pivot;
}

struct tree *rotation(struct tree *root) {
  if(root->bf > 1 && root->left->bf >= 0) { // bf > 1 = left child unbalanced
    return rightRotate(root);
  } else if(root->bf > 1 && root->left->bf < 0) {
    root->left = leftRotate(root->left); // double rotation
    return rightRotate(root);
  } else if(root->bf < -1 && root->right->bf <= 0) { // bf < -1 = right child unbalanced
    return leftRotate(root);
  } else if(root->bf < - 1 && root->right->bf > 0) {
    root->right = rightRotate(root->right); // double rotation
    return leftRotate(root);
  }

  return root;
}

struct tree *insertNode(struct tree *root, int value, const char *name, int score) {
  if(root==NULL) {
    return createNode(value, name, score);
  } else if(value < root->value) {
    root->left = insertNode(root->left, value, name, score);
  } else if(value > root->value) {
    root->right = insertNode(root->right, value, name, score);
  }

  return rotation(updateNode(root));
}

struct tree *predecessor(struct tree *root) {
  struct tree *curr = root->left;

  while(curr->right) {
    curr = curr->right;
  }

  return curr;
}

struct tree *deleteNode(struct tree *root, int value) {
  if(root==NULL) {
    return root;
  } else if(value < root->value) {
    root->left = deleteNode(root->left, value);
  } else if(value > root->value) {
    root->right = deleteNode(root->right, value);
  } else {
    if(root->left==NULL || root->right==NULL) {
      struct tree *newRoot = root->left ? root->left : root->right;
      root->left = root->right = NULL;
      free(root);
      root = NULL;
      return newRoot;
    }

    struct tree *inOrderPredecessor = predecessor(root);
    root->value = inOrderPredecessor->value;
    root->left = deleteNode(root->left, inOrderPredecessor->value);
  }

  return rotation(updateNode(root));
}


void print(struct tree *root){
  if(root) {
  	//preorder
    print(root->left);
    printf("\n-ID            :  %d  \nTinggi         :  %d  \nBalance Factor :  %d  \nNama           :  %s  \nScore          :  %d  \n",root->value,root->height,root->bf,root->name, root->score);
    print(root->right);
  }
}

void postOrder(struct tree *root)
{
	if(root != NULL)
	{
		postOrder(root->left);
		postOrder(root->right);
        printf("\n-ID            :  %d  \nTinggi         :  %d  \nBalance Factor :  %d  \nNama           :  %s  \nScore          :  %d  \n",root->value,root->height,root->bf,root->name, root->score);
	}
}

void preOrder(struct tree *root)
{
	if(root != NULL)
	{
        printf("\n-ID            :  %d  \nTinggi         :  %d  \nBalance Factor :  %d  \nNama           :  %s  \nScore          :  %d  \n",root->value,root->height,root->bf,root->name, root->score);
		preOrder(root->left);
		preOrder(root->right);
	}
}

void printmenu() {

	puts("");
	puts("\t SELECTION OF NEW EMPLOYEE ADMISSION");
	puts("\t ===================================");
	puts("");
	puts("\t [1]. View All Employee");
	puts("\t [2]. Add New Employee");
	puts("\t [3]. Remove Employee From The List");
	puts("\t [4]. Ranking Preview");
	puts("\t [5]. Search");
	puts("\t [6]. Tree Representation");
	puts("\t [7]. Exit");
	puts("");
}

struct tree *searchNode(struct tree *root, int value) {
  if(root) {
    if(value == root->value) {
      return root;
    } else if(value < root->value) {
      return searchNode(root->left, value);
    } else if(value > root->value) {
      return searchNode(root->right, value);
    }
  }

  return 0;
}


// int main() {
//   struct tree *baseRoot = NULL;
//   int toInsert[] = {10, 11, 12, 5, 3, 2, 8, 7, 4};

//   for(int i = 0; i < 9; i++) {
//     baseRoot = insertNode(baseRoot, toInsert[i]);
//     printf("\nAdded %d to the tree.\n", toInsert[i]);
//     printf("| %-3s | %-5s | %-14s |\n", "Key", "Height", "Balance Factor");
//     inOrder(baseRoot); getchar();
//   }

//   for(int i = 0; i < 9; i++) {
//     baseRoot = deleteNode(baseRoot, toInsert[i]);
//     printf("\nRemoved %d from the tree.\n", toInsert[i]);
//     printf("| %-3s | %-5s | %-14s |\n", "Key", "Height", "Balance Factor");
//     inOrder(baseRoot); getchar();
//   }

//   return 0;
// }

int main(){
	int input,input1,value, score;
	char name[255];
	struct tree *root = NULL;
	while(1){
	puts("");
	printmenu();
	printf(">> Input choice :");
	scanf("%d",&input);
		if(input==1){
			if(root==NULL){
				puts("--- There is No Employee in The Tree ---");
			}
			else{
				puts("");
  				puts("Employee List :");
				print(root);
				getchar();
			}
		}
		else if(input==2){
			while(1){
				printf("\nInput Employee' s Name [3..50]: ");
				getchar();
				scanf("%[^\n]s",name);
				getchar();
				if(strlen(name)<3){
                    puts("Name Too Short");
				}else if ( strlen(name)>50){
                    puts("Name Too Long");
                }
				else{
					while(1){
                    printf("Input Employee' s ID [0..100]: ");
					scanf("%d",&value);
					if(value<0 || value>100){
						puts("ID Doesn't Exist");
					}
					printf("Input Employee' s Score [0..100]: ");
					scanf("%d",&score);
					if(score<0 || score>100){
						puts("Score Doesn't Exist");
					}
					else if(searchNode(root,value)==NULL){
						root=insertNode(root,value,name, score);
						break;
					}
					else if(searchNode(root,value)!=NULL){
						puts("Employee's ID Cannot be the Same, Please Input Other ID");
					}
				}
				break;
			}
		}
	}
		else if(input==3){
			while(1){
			printf("Input Employee's ID That You Want to Be Removed[0..100]: ");
			scanf("%d",&value);
		if (root==NULL){
        puts("--- The ID Doesn't Exist ---");
        break;
    	}
	    else{
		root = deleteNode(root, value);
			puts("--- The ID Has Been Removed ---");
			break;
		}
	}
}
		 else if(input==4){
		 	printf("\n1. InOrder");
		 	printf("\n2. PreOrder");
		 	printf("\n3. PostOrder");

		 	printf("\nEnter Your Choice: ");
		 	scanf("%d",&input1);

		 	if(input1==1){
		 		if(root==NULL){
		 			puts("--- There is No ID in The Tree ---");
		 	}else {
		 		printf("\n\nInOrder:");
		 		print(root);
			}
		 	}else if (input1==2){
		 		if(root==NULL){
		 			puts("--- There is No ID in The Tree ---");
		 		}else {
		 			printf("\n\nPreOrder:");
		 			preOrder(root);
		 	}
		 	}else if (input1 == 3){
		 		if(root==NULL){
		 			puts("--- There is No ID in The Tree ---");
		 		}else {
		 			printf("\n\nPostOrder:");
		 			postOrder(root);
		 		}
		 	}
		 }
		 else if (input == 5){
            struct tree *nodeSearchKar = searchNode(root, value);
			printf("\nData Karyawan yang ingin dicari: ");
			scanf("%d",&value);
			printf("\nKaryawan :  %s  \nScore    :  %d \n", nodeSearchKar->name, nodeSearchKar->score);
		 }
		else if(input==6){
			print_ascii_tree(root);
		}
		else if(input==7){
			exit:
			break;
		}
	}
	return 0;
}