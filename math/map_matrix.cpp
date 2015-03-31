/* map matrix class
 * stores a matrix representing a simplicial map
 */

#include "map_matrix.h"

#include <iostream> //for testing only


/********** implementation of base class MapMatrix_Base **********/

//implementation of subclass MapMatrixNode
MapMatrix_Base::MapMatrixNode::MapMatrixNode(unsigned row) :
    row_index(row), next(NULL)
{ }

unsigned MapMatrix_Base::MapMatrixNode::get_row()
{
    return row_index;
}

void MapMatrix_Base::MapMatrixNode::set_next(MapMatrixNode* n)
{
    next = n;
}

MapMatrix_Base::MapMatrixNode* MapMatrix_Base::MapMatrixNode::get_next()
{
    return next;
}

//constructor to create matrix of specified size (all entries zero)
MapMatrix_Base::MapMatrix_Base(unsigned rows, unsigned cols) :
    columns(cols), num_rows(rows)
{ }

//constructor to create a (square) identity matrix
MapMatrix_Base::MapMatrix_Base(unsigned size) :
    columns(size), num_rows(size)
{
    for(unsigned i = 0; i < size; i++)
    {
        MapMatrixNode* newnode = new MapMatrixNode(i);
        columns[i] = newnode;
    }
}

//destructor: deletes all entries in this matrix
MapMatrix_Base::~MapMatrix_Base()
{
    for(unsigned j=0; j<columns.size(); j++)
    {
        MapMatrixNode* current = columns[j];
        while(current != NULL)
        {
            MapMatrixNode* next = current->get_next();
            delete current;
            current = next;
        }
    }
}

//returns the number of columns in the matrix
unsigned MapMatrix_Base::width()
{
    return columns.size();
}

//returns the number of rows in the matrix
unsigned MapMatrix_Base::height()
{
    return num_rows;
}

//sets (to 1) the entry in row i, column j
void MapMatrix_Base::set(unsigned i, unsigned j)
{
    //make sure this operation is valid
    if(columns.size() <= j)
        throw std::runtime_error("attempting to set column past end of matrix");
    if(num_rows <= i)
        throw std::runtime_error("attempting to set row past end of matrix");

    //if the column is empty, then create a node
    if(columns[j] == NULL)
    {
        columns[j] = new MapMatrixNode(i);
        return;
    }

    //if we get here, the column is not empty
    MapMatrixNode* current = columns[j];

    //see if node that we would insert already exists in the first position
    if(current->get_row() == i)
    {
        return;	//avoid duplicate nodes
    }

    //see if we need to insert a new node into the first position
    if(current->get_row() < i)
    {
        MapMatrixNode* newnode = new MapMatrixNode(i);
        newnode->set_next(current);
        columns[j] = newnode;
        return;
    }

    //if we get here, then we must traverse the nodes to find the proper insertion point
    while(current->get_next() != NULL)
    {
        MapMatrixNode* next = current->get_next();

        if(next->get_row() == i)	//then node aready exists
        {
            return;	//avoid duplicate nodes
        }

        if(next->get_row() < i) //then insert new node between current and next
        {
            MapMatrixNode* newnode = new MapMatrixNode(i);
            newnode->set_next(next);
            current->set_next(newnode);

            return;
        }

        //otherwise, move one step
        current = next;
    }

    //if we get here, then append a new node to the end of the list
    MapMatrixNode* newnode = new MapMatrixNode(i);
    current->set_next(newnode);
}//end set()

//clears (sets to 0) the entry in row i, column j
void MapMatrix_Base::clear(unsigned i, unsigned j)
{
    //make sure this entry is valid
    if(columns.size() <= j)
        throw std::runtime_error("attempting to clear entry in a column past end of matrix");
    if(num_rows <= i)
        throw std::runtime_error("attempting to clear entry in a row row past end of matrix");

    //if column is empty, then do nothing
    if(columns[j] == NULL)
        return;

    //column is not empty, so get initial node pointer in the column
    MapMatrixNode* current = columns[j];

    //see if the first node is the one we want
    if(current->get_row() < i)
        return;     //because the entry in row i must be zero (row entries are sorted in decreasing order)

    if(current->get_row() == i)
    {
        columns[j] = current->get_next();
        delete current;
        return;
    }

    //traverse the nodes in this column after the first node
    while(current->get_next() != NULL)
    {
        MapMatrixNode* next = current->get_next();

        if(next->get_row() < i)	//then the entry in row i must be zero (row entries are sorted in decreasing order)
            return;

        if(next->get_row() == i) //then we found the row we wanted
        {
            current->set_next(next->get_next());
            delete next;
            return;
        }

        //if we are still looking, then get the next node
        current = next;
    }
}//end clear()

//returns true if entry (i,j) is 1, false otherwise
bool MapMatrix_Base::entry(unsigned i, unsigned j)
{
    //make sure this entry is valid
    if(columns.size() <= j)
        throw std::runtime_error("attempting to check entry in a column past end of matrix");
    if(num_rows <= i)
        throw std::runtime_error("attempting to check entry in a row row past end of matrix");

    //get initial node pointer
    MapMatrixNode* np = columns[j];

    //loop while there is another node to check
    while(np != NULL)
    {
        if(np->get_row() < i)	//then we won't find row i because row entrys are sorted in descending order
            return false;

        if(np->get_row() == i) //then we found the row we wanted
            return true;

        //if we are still looking, then get the next node
        np = np->get_next();
    }

    //if we get here, then we didn't find the entry
    return false;
}//end entry()

//adds column j to column k
//  RESULT: column j is not changed, column k contains sum of columns j and k (with mod-2 arithmetic)
void MapMatrix_Base::add_column(unsigned j, unsigned k)
{
    //make sure this operation is valid
    if(columns.size() <= j || columns.size() <= k)
        throw std::runtime_error("attempting to access column past end of matrix");
    if(j == k)
        throw std::runtime_error("adding a column to itself");

    //pointers
    MapMatrixNode* jnode = columns[j]; //points to next node from column j that we will add to column k
    MapMatrixNode* khandle = NULL; //points to node in column k that was most recently added; will be non-null after first node is added

    //loop through all entries in column j
    while(jnode != NULL)
    {
        //now it is save to dereference jnode (*jnode)...
        unsigned row = jnode->get_row();

        //loop through entries in column k, starting at the current position
        bool added = false;

        if(columns[k] == NULL) //then column k is empty, so insert initial node
        {
            MapMatrixNode* newnode = new MapMatrixNode(row);
            columns[k] = newnode;
            khandle = newnode;

            added = true; //proceed with next element from column j
        }

        if(!added && khandle == NULL) //then we haven't yet added anything to column k (but if we get here, column k is nonempty)
        {
            if((*columns[k]).get_row() == row) //then remove this node (since 1+1=0)
            {
                MapMatrixNode* next = (*columns[k]).get_next();
                delete columns[k];
                columns[k] = next;
                added = true; //proceed with next element from column j
            }
            else if((*columns[k]).get_row() < row) //then insert new initial node into column k
            {
                MapMatrixNode* newnode = new MapMatrixNode(row);
                newnode->set_next(columns[k]);
                columns[k] = newnode;
                khandle = columns[k];
                added = true; //proceed with next element from column j
            }
            else //then move to next node in column k
            {
                khandle = columns[k];
                //now we want to enter the following while loop
            }
        }//end if

        while(!added && ( khandle->get_next() != NULL )) //if we get here, both columns[k] and khandle are NOT NULL
        {
            //consider the next node
            MapMatrixNode* next = khandle->get_next();

            if(next->get_row() == row) //then remove the next node (since 1+1=0)
            {
                khandle->set_next( next->get_next() );
                delete next;
                added = true; //proceed with next element from column j
            }
            else if(next->get_row() < row) //then insert new initial node into column k
            {
                MapMatrixNode* newnode = new MapMatrixNode(row);
                newnode->set_next(next);
                khandle->set_next(newnode);
                added = true; //proceed with next element from column j
            }
            else //then next->get_row() > row, so move to next node in column k
            {
                khandle = next;
            }
        }//end while

        if(!added && ( khandle->get_next() == NULL )) //then we have reached the end of the list, and we should append a new node
        {
            MapMatrixNode* newnode = new MapMatrixNode(row);
            khandle->set_next(newnode);
            khandle = newnode;
        }

        //move to the next entry in column j
        jnode = jnode->get_next();
    }//end while(jnode != NULL)

}//end add_column()



/********** implementation of class MapMatrix, for column-sparse matrices **********/

//constructor that sets initial size of matrix
MapMatrix::MapMatrix(unsigned rows, unsigned cols) :
    MapMatrix_Base(rows, cols)
{ }

//constructor to create a (square) identity matrix
MapMatrix::MapMatrix(unsigned size) :
    MapMatrix_Base(size)
{ }

//destructor: deletes all entries in this matrix (via automatic call to destructor of MapMatrix_Base)
MapMatrix::~MapMatrix()
{ }

//returns the number of columns in the matrix
unsigned MapMatrix::width()
{
    return MapMatrix_Base::width();
}

//returns the number of rows in the matrix
unsigned MapMatrix::height()
{
    return MapMatrix_Base::height();
}

//sets (to 1) the entry in row i, column j
void MapMatrix::set(unsigned i, unsigned j)
{
    MapMatrix_Base::set(i, j);
}

//returns true if entry (i,j) is 1, false otherwise
bool MapMatrix::entry(unsigned i, unsigned j)
{
    return MapMatrix_Base::entry(i, j);
}

//returns the "low" index in the specified column, or 0 if the column is empty or does not exist
int MapMatrix::low(unsigned j)
{
    //make sure this query is valid
    if(columns.size() <= j)
        throw std::runtime_error("attempting to check low number of a column past end of matrix");
	
    //if the column is empty, then return -1
    if(columns[j] == NULL)
        return -1;
	
    //otherwise, column is non-empty, so return first index (because row indexes are sorted in descending order)
    return (*columns[j]).get_row();
}

//returns true iff column j is empty
bool MapMatrix::col_is_empty(unsigned j)
{
    return (columns[j] == NULL);
}

//adds column j to column k; RESULT: column j is not changed, column k contains sum of columns j and k (with mod-2 arithmetic)
void MapMatrix::add_column(unsigned j, unsigned k)
{
    MapMatrix_Base::add_column(j, k);
}

//adds column j from MapMatrix* other to column k of this matrix
void MapMatrix::add_column(MapMatrix* other, unsigned j, unsigned k)
{
    //make sure this operation is valid
    if(other->columns.size() <= j || columns.size() <= k)
        throw std::runtime_error("attempting to access column(s) past end of matrix");

    //pointers
    MapMatrixNode* jnode = other->columns[j]; //points to next node from column j that we will add to column k
    MapMatrixNode* khandle = NULL; //points to node in column k that was most recently added; will be non-null after first node is added

    //loop through all entries in column j
    while(jnode != NULL)
    {
        //now it is save to dereference jnode
        unsigned row = jnode->get_row();

        //loop through entries in column k, starting at the current position
        bool added = false;

        if(columns[k] == NULL) //then column k is empty, so insert initial node
        {
            MapMatrixNode* newnode = new MapMatrixNode(row);
            columns[k] = newnode;
            khandle = newnode;

            added = true; //proceed with next element from column j
        }

        if(!added && khandle == NULL) //then we haven't yet added anything to column k (but if we get here, column k is nonempty)
        {
            if((*columns[k]).get_row() == row) //then remove this node (since 1+1=0)
            {
                MapMatrixNode* next = (*columns[k]).get_next();
                delete columns[k];
                columns[k] = next;
                added = true; //proceed with next element from column j
            }
            else if((*columns[k]).get_row() < row) //then insert new initial node into column k
            {
                MapMatrixNode* newnode = new MapMatrixNode(row);
                newnode->set_next(columns[k]);
                columns[k] = newnode;
                khandle = columns[k];
                added = true; //proceed with next element from column j
            }
            else //then move to next node in column k
            {
                khandle = columns[k];
                //now we want to enter the following while loop
            }
        }//end if

        while(!added && ( khandle->get_next() != NULL )) //if we get here, both columns[k] and khandle are NOT NULL
        {
            //consider the next node
            MapMatrixNode* next = khandle->get_next();

            if(next->get_row() == row) //then remove the next node (since 1+1=0)
            {
                khandle->set_next( next->get_next() );
                delete next;
                added = true; //proceed with next element from column j
            }
            else if(next->get_row() < row) //then insert new initial node into column k
            {
                MapMatrixNode* newnode = new MapMatrixNode(row);
                newnode->set_next(next);
                khandle->set_next(newnode);
                added = true; //proceed with next element from column j
            }
            else //then next->get_row() > row, so move to next node in column k
            {
                khandle = next;
            }
        }//end while

        if(!added && ( khandle->get_next() == NULL )) //then we have reached the end of the list, and we should append a new node
        {
            MapMatrixNode* newnode = new MapMatrixNode(row);
            khandle->set_next(newnode);
            khandle = newnode;
        }

        //move to the next entry in column j
        jnode = jnode->get_next();
    }//end while(jnode != NULL)

}//end add_column(MapMatrix*, unsigned, unsigned)

//applies the column reduction algorithm to this matrix
void MapMatrix::col_reduce()
{
    //create low array
    std::vector<int> lows(num_rows, -1);

    //loop through columns
    for(unsigned j=0; j < columns.size(); j++)
    {
        //while column j is nonempty and its low number is found in the low array, do column operations
        while(low(j) >= 0 && lows[low(j)] >= 0)
            add_column(lows[low(j)], j);

        if(low(j) >= 0)    //then column is still nonempty, so update lows
            lows[low(j)] = j;
    }
}//end col_reduce()

//function to print the matrix to standard output, for testing purposes
void MapMatrix::print()
{
    //handle empty matrix
	if(num_rows == 0 || columns.size() == 0)
	{
        std::cout << "        (empty matrix: " << num_rows << " rows by " << columns.size() << " columns)\n";
		return;
	}
	
	//create a 2D array of booleans to temporarily store the matrix
	bool mx[num_rows][columns.size()];
    for(unsigned i=0; i<num_rows; i++)
        for(unsigned j=0; j<columns.size(); j++)
			mx[i][j] = false;
	
	//traverse the linked lists in order to fill the 2D array
	MapMatrixNode* current;
    for(unsigned j=0; j<columns.size(); j++)
	{
		current = columns[j];
		while(current != NULL)
		{
            int row = current->get_row();
			mx[row][j] = 1;
            current = current->get_next();
		}
	}
	
	//print the matrix
    for(unsigned i=0; i<num_rows; i++)
	{
		std::cout << "        |";
        for(unsigned j=0; j<columns.size(); j++)
		{
			if(mx[i][j])
				std::cout << " 1";
			else
				std::cout << " 0";
		}
		std::cout << " |\n";
	}
}//end print()



/********** implementation of class MapMatrix_Perm, supports row swaps (and stores a low array) **********/

MapMatrix_Perm::MapMatrix_Perm(unsigned rows, unsigned cols) :
    MapMatrix(rows, cols),
    perm(rows), mrep(rows), low_col(rows, -1), col_perm(cols)
{
    //initialize permutation vectors to the identity permutation
    for(unsigned i=0; i < rows; i++)
    {
        perm[i] = i;
        mrep[i] = i;
    }

    for(unsigned j=0; j < cols; j++)
        col_perm[j] = j;
}

MapMatrix_Perm::MapMatrix_Perm(unsigned size) :
    MapMatrix(size),
    perm(size), mrep(size), low_col(size, -1)
{
    //initialize permutation vectors to the identity permutation
    for(unsigned i=0; i < size; i++)
    {
        perm[i] = i;
        mrep[i] = i;
    }
}

MapMatrix_Perm::~MapMatrix_Perm()
{ }

//sets (to 1) the entry in row i, column j
//NOTE: to be used for matrix construction only; does not update low array
void MapMatrix_Perm::set(unsigned i, unsigned j)
{
    MapMatrix::set(mrep[i], j);
}

//returns true if entry (i,j) is 1, false otherwise
bool MapMatrix_Perm::entry(unsigned i, unsigned j)
{
    return MapMatrix::entry(mrep[i], j);
}

//returns the "low" index in the specified column, or -1 if the column is empty
//NOTE: this is potentially slow, but I don't know how to improve it (we could store the low number with each column, but then we would have to keep this data up-to-date)
//written to avoid comparing int and unsigned types
int MapMatrix_Perm::low(unsigned j)
{
    //make sure this query is valid
    if(columns.size() <= j)
        throw std::runtime_error("attempting to check low number of a column past end of matrix");

    //return -1 if the column is empty
    if(columns[j] == NULL)
        return -1;

    //consider the first node
    MapMatrixNode* current = columns[j];
    unsigned lowest = perm[current->get_row()];

    //consider all following nodes
    current = current->get_next();
    while(current != NULL)
    {
        if(perm[current->get_row()] > lowest)
            lowest = perm[current->get_row()];

        current = current->get_next();
    }

    return lowest;
}

//returns the index of the column with low l, or -1 if there is no such column
int MapMatrix_Perm::find_low(unsigned l)
{
    ///THIS IS AWFUL, BUT LOW ARRAY IS BROKEN
    for(int j=0; j<columns.size(); j++)
    {
        if(low(j) == l)
            return j;
    }
    return -1;

///RESTORE THIS WHEN USING LOW ARRAY!!!
//    return low_col[l];
}

//transposes rows i and i+1
//NOTE: this causes low array to be incorrect iff there are columns k and l with low(k)=i, low(l)=i+1, and M[i,l]=1  (as in Vineyards, Case 1.1)
//      the user must detect this and do a column operation to restore the matrix to a reduced state!
void MapMatrix_Perm::swap_rows(unsigned i)
{
    //get original row indexes of these rows
    unsigned a = mrep[i];
    unsigned b = mrep[i+1];

    //swap perm[a] and perm[b]
    unsigned temp = perm[a];
    perm[a] = perm[b];
    perm[b] = temp;

    //swap mrep[i] and mrep[i+1]
    mrep[i] = b;
    mrep[i+1] = a;

///RESTORE THIS WHEN USING LOW ARRAY!!!
//    //also swap entries in low array
//    int t = low_col[i];
//    low_col[i] = low_col[i+1];
//    low_col[i+1] = t;
}//end swap_rows()

//transposes columns j and j+1
void MapMatrix_Perm::swap_columns(unsigned j)
{
    //swap columns
    MapMatrixNode* temp = columns[j];
    columns[j] = columns[j+1];
    columns[j+1] = temp;

///RESTORE THIS WHEN USING LOW ARRAY!!!
//    //also swap entries in low array
//    int a = low(j);
//    if(a != -1)
//        low_col[a] = j;
//    int b = low(j+1);
//    if(b != -1)
//        low_col[b] = j+1;

    ///TESTING ONLY
    unsigned a = col_perm[j];
    col_perm[j] = col_perm[j+1];
    col_perm[j+1] = a;
}

//reduces this matrix and returns the corresponding upper-triangular matrix for the RU-decomposition
MapMatrix_RowPriority_Perm* MapMatrix_Perm::decompose_RU()
{
    //create the matrix U
    MapMatrix_RowPriority_Perm* U = new MapMatrix_RowPriority_Perm(columns.size());   //NOTE: must be deleted later!

    //loop through columns
    for(unsigned j=0; j<columns.size(); j++)
    {
        //while column j is nonempty and its low number is found in the low array, do column operations
        while(low(j) >= 0 && low_col[low(j)] >= 0)
        {
            int c = low_col[low(j)];
            add_column(c, j);
            U->add_row(j, c);  //perform the opposite row operation on U
        }

        if(low(j) >= 0)    //then column is still nonempty, so update lows
            low_col[low(j)] = j;
    }

    //return the matrix U
    return U;
}//end decompose_RU()

//function to print the matrix to standard output, for testing purposes
void MapMatrix_Perm::print()
{
    //print permutation
    std::cout << "    row permutation: ";
    for(unsigned i=0; i<num_rows; i++)
        std::cout << perm[i] << " ";
    std::cout << "\n";
    std::cout << "    INVERSE column permutation: ";
    for(unsigned j=0; j<columns.size(); j++)
        std::cout << col_perm[j] << " ";
    std::cout << "\n";

    //handle empty matrix
    if(num_rows == 0 || columns.size() == 0)
    {
        std::cout << "        (empty matrix: " << num_rows << " rows by " << columns.size() << " columns)\n";
        return;
    }

    //create a 2D array of booleans to temporarily store the matrix
    bool mx[num_rows][columns.size()];
    for(unsigned i=0; i<num_rows; i++)
        for(unsigned j=0; j<columns.size(); j++)
            mx[i][j] = false;

    //traverse the linked lists in order to fill the 2D array
    MapMatrixNode* current;
    for(unsigned j=0; j<columns.size(); j++)
    {
        current = columns[j];
        while(current != NULL)
        {
            int row = current->get_row();
            mx[perm[row]][j] = 1;
            current = current->get_next();
        }
    }

    //print the matrix
    for(unsigned i=0; i<num_rows; i++)
    {
        std::cout << "        |";
        for(unsigned j=0; j<columns.size(); j++)
        {
            if(mx[i][j])
                std::cout << " 1";
            else
                std::cout << " 0";
        }
        std::cout << " |\n";
    }
}//end print()



/********** implementation of class MapMatrix_RowPriority_Perm **********/

MapMatrix_RowPriority_Perm::MapMatrix_RowPriority_Perm(unsigned size) :
    MapMatrix_Base(size),
    perm(size), mrep(size)
{
    //initialize permutation vectors to the identity permutation
    for(unsigned i=0; i < size; i++)
    {
      perm[i] = i;
      mrep[i] = i;
    }
}

MapMatrix_RowPriority_Perm::~MapMatrix_RowPriority_Perm()
{ }

unsigned MapMatrix_RowPriority_Perm::width()
{
    return MapMatrix_Base::height();
}

unsigned MapMatrix_RowPriority_Perm::height()
{
    return MapMatrix_Base::width();
}

void MapMatrix_RowPriority_Perm::set(unsigned i, unsigned j)
{
    MapMatrix_Base::set(mrep[j], i);
}

void MapMatrix_RowPriority_Perm::clear(unsigned i, unsigned j)
{
    MapMatrix_Base::clear(mrep[j], i);
}

bool MapMatrix_RowPriority_Perm::entry(unsigned i, unsigned j)
{
    return MapMatrix_Base::entry(mrep[j], i);
}

//adds row j to row k; RESULT: row j is not changed, row k contains sum of rows j and k (with mod-2 arithmetic)
void MapMatrix_RowPriority_Perm::add_row(unsigned j, unsigned k)
{
    return MapMatrix_Base::add_column(j, k);
}

//transposes rows i and i+1
void MapMatrix_RowPriority_Perm::swap_rows(unsigned i)
{
    MapMatrixNode* temp = columns[i];
    columns[i] = columns[i+1];
    columns[i+1] = temp;
}

//transposes columns j and j+1
void MapMatrix_RowPriority_Perm::swap_columns(unsigned j)
{
    //get original indexes of these columns
    unsigned a = mrep[j];
    unsigned b = mrep[j+1];

    //swap perm[a] and perm[b]
    unsigned temp = perm[a];
    perm[a] = perm[b];
    perm[b] = temp;

    //swap mrep[i] and mrep[i+1]
    mrep[j] = b;
    mrep[j+1] = a;
}

//prints the matrix to stadard output, for testing
//this function is identical to MapMatrix::print(), with rows and columns transposed
void MapMatrix_RowPriority_Perm::print()
{
    //handle empty matrix
    if(num_rows == 0 || columns.size() == 0)
    {
        std::cout << "        (empty matrix: " << columns.size() << " rows by " << num_rows << " columns)\n";
        return;
    }

    //create a 2D array of booleans to temporarily store the matrix
    bool mx[columns.size()][num_rows];
    for(unsigned i=0; i<columns.size(); i++)
        for(unsigned j=0; j<num_rows; j++)
            mx[i][j] = false;

    //traverse the linked lists in order to fill the 2D array
    MapMatrixNode* current;
    for(unsigned j=0; j<columns.size(); j++)
    {
        current = columns[j];
        while(current != NULL)
        {
            unsigned row = current->get_row();
            mx[j][perm[row]] = 1;
            current = current->get_next();
        }
    }

    //print the matrix
    for(unsigned i=0; i<num_rows; i++)
    {
        std::cout << "        |";
        for(unsigned j=0; j<columns.size(); j++)
        {
            if(mx[i][j])
                std::cout << " 1";
            else
                std::cout << " 0";
        }
        std::cout << " |\n";
    }
}//end print()
