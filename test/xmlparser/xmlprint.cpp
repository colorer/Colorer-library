
#include<windows.h>
#include<stdio.h>

#define COLOR(c) SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),c)
#define BR    0x0E
#define TAG   0x0F
#define NORM  0x07
#define PAR   0x0C
#define PLAIN 0x09

void printLevel(Node *node, int lev)
{
  int i;
  Node *tmp = node;

  do{

    if (tmp->getNodeType() == Node::DOCUMENT_NODE){
      Node *elem = ((Document*)tmp)->getFirstChild();
      if (elem){
        printLevel(elem, lev);
      }
    }

    if (tmp->getNodeType() == Node::ELEMENT_NODE){
      Element *elem = (Element*)tmp;

      for(i = 0; i < lev*3; i++){
        printf(" ");
      }
      COLOR(BR);
      printf("<");
      COLOR(TAG);
      printf("%s", elem->getNodeName()->getChars());
      COLOR(NORM);

      const Vector<const String*> *attrs = elem->getAttributes();
      for (i = 0; i < attrs->size(); i++){
        COLOR(PAR);
        printf(" %s=", attrs->elementAt(i)->getChars());
        COLOR(NORM);
        printf("\"%s\"", elem->getAttribute(attrs->elementAt(i))->getChars());
      };
      COLOR(BR);
      printf(">\n");

      if (tmp->hasChildNodes()){
        printLevel(elem->getFirstChild(), lev+1);
      };

      for(i = 0; i < lev*3; i++) printf(" ");

      COLOR(BR);  printf("</");
      COLOR(TAG);  printf("%s", elem->getNodeName()->getChars());
      COLOR(BR);  printf(">\n");
      COLOR(NORM);
    }

    if (tmp->getNodeType() == Node::TEXT_NODE){
      Text *elem = (Text*)tmp;
      COLOR(PLAIN);
      printf("%s", elem->getData()->getChars());
      COLOR(NORM);

      CharacterData *elem1 = (CharacterData*)elem;
      elem1->getData();
    };

    if (tmp->getNodeType() == Node::PROCESSING_INSTRUCTION_NODE){
      ProcessingInstruction *elem = (ProcessingInstruction*)tmp;
      COLOR(PLAIN);
      printf("<?%s %s?>", elem->getTarget()->getChars(), elem->getData()->getChars());
      COLOR(NORM);

      CharacterData *elem1 = (CharacterData*)elem;
      elem1->getData();

    };

    if (tmp->getNodeType() == Node::COMMENT_NODE){
      Comment *elem = (Comment*)tmp;
      COLOR(PLAIN);
      printf("<!--%s-->", elem->getData()->getChars());
      elem->getLength();
      COLOR(NORM);
    };

    tmp = tmp->getNextSibling();
  }while(tmp);
};
