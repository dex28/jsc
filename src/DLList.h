#ifndef __DLLIST_HPP
#define __DLLIST_HPP

/*
Base classes for doubly linked list implementations. The
DNodeBase class and the DLListBase class separates the nodes
from the data stored in the list by storing pointer to the
data. Classes derived from these base classes can deal with
any type of node data.
*/

// Doubly Linked List Node Base class
//
class DNodeBase
{
protected:
  friend class DLListBase;

protected:
  void InsertAfter(DNodeBase *Node);
  void InsertBefore(DNodeBase *Node);
  DNodeBase *Rmv();
  void SelfRef() { Prior = this; Next = this; }

protected:
  DNodeBase *Prior;
  DNodeBase *Next;
};

// Doubly Linked List Base class
//
class DLListBase : public DNodeBase
{
protected:
  DLListBase() { MakeEmpty(); }
  virtual ~DLListBase(); 

public:
  virtual void Clear();
  int IsEmpty() const { return (const DNodeBase *)Next == this; }
  int IsHeader(const DNodeBase *Node) const { return Node == this; }
  
protected:
  virtual DNodeBase *DupNode(const DNodeBase *Node) = 0;
  virtual void FreeNode(DNodeBase *Node) = 0;
  virtual void MakeEmpty(); 
  DNodeBase *GetHeader() { return this; }
  const DNodeBase *GetHeader() const { return this; }
  DNodeBase *GetFront() { return Next; }
  const DNodeBase *GetFront() const { return Next; }
  DNodeBase *GetBack() { return Prior; }
  const DNodeBase *GetBack() const { return Prior; }
  void InsertBefore(DNodeBase *A, DNodeBase *B) {
    A->InsertBefore(B); }
  void InsertAfter(DNodeBase *A, DNodeBase *B) {
    A->InsertAfter(B); }
  void AttachToFront(DNodeBase *Node) { InsertAfter(this, Node); }
  void AttachToBack(DNodeBase *Node) { InsertAfter(Prior, Node); }
  DNodeBase *Rmv(DNodeBase *Node);
  DNodeBase *RmvFront() { return Rmv(Next); }
  DNodeBase *RmvBack() { return Rmv(Prior); }
  int Copy(const DLListBase &List);
  int Cat(const DLListBase &List);
};

// DLLIST.CPP -----------------------------------------------------------------

inline void DNodeBase::InsertBefore(DNodeBase *Node)
{
  Node->Prior = Prior;
  Prior->Next = Node;
  Node->Next = this;
  Prior = Node;
}

inline void DNodeBase::InsertAfter(DNodeBase *Node)
{
  Node->Next = Next;
  Next->Prior = Node;
  Node->Prior = this;
  Next = Node;
}

inline DNodeBase *DNodeBase::Rmv()
{
  Prior->Next = Next;
  Next->Prior = Prior;
  return this;
}

inline DLListBase::~DLListBase()
{
  // Destructor provided for virtuality
}

inline void DLListBase::MakeEmpty()
{
  SelfRef(); 
}

inline void DLListBase::Clear()
{
  DNodeBase *Node = Next;
  while(!IsHeader(Node)) {
    DNodeBase *NextNode = Node->Next;
    FreeNode(Node); // Must be defined in derived  classes
    Node = NextNode;
  }
  MakeEmpty();
}

inline DNodeBase *DLListBase::Rmv(DNodeBase *Node)
{
  if(Node == this) return 0; // Return null if Node is the header
  return Node->Rmv(); // Remove and return Node pointer
}

inline int DLListBase::Copy(const DLListBase &List)
{
  if(&List == this) return 1; // Already its own copy
  Clear(); // Clear current nodes from this list
  return Cat(List); // 1 if successful, 0 if fails
}

inline int DLListBase::Cat(const DLListBase &List)
{
  if(this == &List) return 0; // Do not append list onto itself
  const DNodeBase *ptr = List.Next;
  while(!List.IsHeader(ptr)) { // For all nodes in List
    DNodeBase *Node = DupNode(ptr);
    if(Node == 0) return 0; // Incomplete copy made
    AttachToBack(Node);
    ptr = ptr->Next;
  }
  return 1; // Successful concatenation
}    

// ----------------------------------------------------------- //

/*
A generic doubly linked list class derived from the DNodeBase
class and the DLListBase class.
*/

// Doubly Linked List Node class 
//
template<class TYPE>
class DNode : public DNodeBase
{
public:
  DNode() { } // Implicitly call default constructor for Data
  DNode(const TYPE &X) : Data(X) { } // Call copy constructor

public:
  DNode<TYPE> *GetPrior() { return (DNode<TYPE> *)Prior; }
  const DNode<TYPE> *GetPrior() const { return (DNode<TYPE> *)Prior; }
  DNode<TYPE> *GetNext() { return (DNode<TYPE> *)Next; }
  const DNode<TYPE> *GetNext() const { return (DNode<TYPE> *)Next; }

public:
  TYPE Data;
};

// Doubly Linked List class
//
template<class TYPE>
class DLList : public DLListBase
{
public:
  DLList() { }
  virtual ~DLList();
  DLList(const DLList<TYPE> &X);
  void operator=(const DLList<TYPE> &X);

public:
  int Copy(const DLList<TYPE> &List);
  int Cat(const DLList<TYPE> &X) { return DLListBase::Cat(X); }
  const DNode<TYPE> *Find(const TYPE &X, const DNode<TYPE> *ptr=0) const;
  DNode<TYPE> *Find(const TYPE &X, DNode<TYPE> *ptr=0);
  int Delete(DNode<TYPE> *Node, TYPE *X = 0);
  int DeleteFront(TYPE *X = 0);
  int DeleteBack(TYPE *X = 0);
  DNode<TYPE> *StoreNode(const TYPE &X);
  DNode<TYPE> *AddToFront(const TYPE &X);
  DNode<TYPE> *AddToBack(const TYPE &X);
  DNode<TYPE> *AddBefore(const TYPE &X, DNode<TYPE> *Node);
  DNode<TYPE> *AddAfter(const TYPE &X, DNode<TYPE> *Node);
  DNode<TYPE> *GetHeader() { return(DNode<TYPE> *) this; }
  const DNode<TYPE> *GetHeader() const { return(DNode<TYPE> *) this; }
  DNode<TYPE> *GetFront() { return(DNode<TYPE> *)DLListBase::GetFront(); }
  DNode<TYPE> *GetBack() { return (DNode<TYPE> *)DLListBase::GetBack(); }
  
  const DNode<TYPE> *GetFront() const { // Read only version
    return(DNode<TYPE> *)DLListBase::GetFront();
  }

  const DNode<TYPE> *GetBack() const { // Read only version
    return (DNode<TYPE> *)DLListBase::GetBack();
  }

  int IsHeader(const DNode<TYPE> *Node) const {
    return DLListBase::IsHeader(Node);
  }
  TYPE *GetFrontNode() {
    DNode<TYPE> *ptr = (DNode<TYPE> *)DLListBase::GetFront();
    return IsHeader(ptr) ? 0 : &(ptr->Data);
  }

  const TYPE *GetFrontNode() const {
    DNode<TYPE> *ptr = (DNode<TYPE> *)DLListBase::GetFront();
    return IsHeader(ptr) ? 0 : &(ptr->Data);
  }

  TYPE *GetBackNode() {
    DNode<TYPE> *ptr = (DNode<TYPE> *)DLListBase::GetBack();
    return IsHeader(ptr) ? 0 : &(ptr->Data);
  }

  const TYPE *GetBackNode() const {
    DNode<TYPE> *ptr = (DNode<TYPE> *)DLListBase::GetBack();
    return IsHeader(ptr) ? 0 : &(ptr->Data);
  }

public:
  void InsertBefore(DNode<TYPE> *A, DNode<TYPE> *B) {
    DLListBase::InsertBefore(A, B);
  }

  void InsertAfter(DNode<TYPE> *A, DNode<TYPE> *B) {
    DLListBase::InsertAfter(A, B);
  }

  void AttachToFront(DNode<TYPE> *Node) {
    DLListBase::AttachToFront(Node);
  }

  void AttachToBack(DNode<TYPE> *Node) {
    DLListBase::AttachToBack(Node);
  }

  DNode<TYPE> *RmvFront() {
    return(DNode<TYPE> *)(DLListBase::RmvFront());
  }

  DNode<TYPE> *Rmv(DNode<TYPE> *Node) {
    return(DNode<TYPE> *)(DLListBase::Rmv(Node));
  }

  DNode<TYPE> *RmvBack() {
    return(DNode<TYPE> *)(DLListBase::RmvBack());
  }

protected:
  virtual DNode<TYPE> *AllocNode(const TYPE &X);
  virtual DNodeBase *DupNode(const DNodeBase *Node);
  virtual void FreeNode(DNodeBase *Node);

public: // Overloaded operators
  int operator+=(const DLList<TYPE> &X) { return Cat(X); }
};

template<class TYPE>
DLList<TYPE>::~DLList()
{
  Clear();
}

template<class TYPE>
DLList<TYPE>::DLList(const DLList<TYPE> &X)
{
  Copy(X);
}

template<class TYPE>
void DLList<TYPE>::operator=(const DLList<TYPE> &X)
{
  Copy(X);
}

template<class TYPE>
int DLList<TYPE>::Copy(const DLList<TYPE> &List)
{
  return DLListBase::Copy(List);
}

template<class TYPE>
DNode<TYPE> *DLList<TYPE>::AllocNode(const TYPE &X)
{
  return NEW DNode<TYPE>(X);
}

template<class TYPE>
DNodeBase *DLList<TYPE>::DupNode(const DNodeBase *Node)
{
  return AllocNode(((DNode<TYPE> *)Node)->Data);
}

template<class TYPE>
void DLList<TYPE>::FreeNode(DNodeBase *Node)
{
  delete((DNode<TYPE> *)Node);
}

template<class TYPE>
const DNode<TYPE> *DLList<TYPE>::
Find(const TYPE &X, const DNode<TYPE> *ptr) const
// Returns the first node having an element that matched X
{
  if(ptr == 0) ptr = GetFront();

  while(!IsHeader(ptr)) { // Scan until end of list
    if(ptr->Data == X) return ptr; // Match found
    ptr = ptr->GetNext();
  }
  return 0; // No match
}

template<class TYPE>
DNode<TYPE> *DLList<TYPE>::Find(const TYPE &X, DNode<TYPE> *ptr)
// Returns the first node having an element that matched X
{
  if(ptr == 0) ptr = GetFront();

  while(!IsHeader(ptr)) { // Scan until end of list
    if(ptr->Data == X) return ptr; // Match found
    ptr = ptr->GetNext();
  }
  return 0; // No match
}

template<class TYPE>
int DLList<TYPE>::Delete(DNode<TYPE> *Node, TYPE *X)
{
  DNode<TYPE> *ptr = Rmv(Node);
  if(ptr) {
    if(X) *X = ptr->Data; // Copy Data into X if X != 0
    FreeNode(ptr);
    return 1; // Return 1 if successful
  }
  return 0; 
}

template<class TYPE>
int DLList<TYPE>::DeleteFront(TYPE *X)
{
  DNode<TYPE> *ptr = RmvFront();
  if(ptr) {
    if(X) *X = ptr->Data; // Copy Data into X if X != 0
    FreeNode(ptr);
    return 1; // Return 1 if successful
  }
  return 0; 
}

template<class TYPE>
int DLList<TYPE>::DeleteBack(TYPE *X)
{
  DNode<TYPE> *ptr = RmvBack();
  if(ptr) {
    if(X) *X = ptr->Data; // Copy Data into X if X != 0
    FreeNode(ptr);
    return 1; // Return 1 if successful
  }
  return 0; 
}

template<class TYPE>
DNode<TYPE> *DLList<TYPE>::StoreNode(const TYPE &X)
{
  DNode<TYPE> *ptr = AllocNode(X);
  if(ptr) AttachToBack(ptr);
  return ptr; // Return a pointer to the node added
}

template<class TYPE>
DNode<TYPE> *DLList<TYPE>::AddToFront(const TYPE &X)
{
  DNode<TYPE> *ptr = AllocNode(X);
  if(ptr) AttachToFront(ptr);
  return ptr; // Return a pointer to the node added
}

template<class TYPE>
DNode<TYPE> *DLList<TYPE>::AddToBack(const TYPE &X)
{
  DNode<TYPE> *ptr = AllocNode(X);
  if(ptr) AttachToBack(ptr);
  return ptr; // Return a pointer to the node added
}

template<class TYPE>
DNode<TYPE> *DLList<TYPE>::AddBefore(const TYPE &X, DNode<TYPE> *Node)
{
  DNode<TYPE> *ptr = AllocNode(X);
  if(ptr) InsertBefore(Node, ptr);
  return ptr; // Return a pointer to the node added
}

template<class TYPE>
DNode<TYPE> *DLList<TYPE>::AddAfter(const TYPE &X, DNode<TYPE> *Node)
{
  DNode<TYPE> *ptr = AllocNode(X);
  if(ptr) InsertAfter(Node, ptr);
  return ptr; // Return a pointer to the node added
}

#endif  // __DLLIST_HPP 
