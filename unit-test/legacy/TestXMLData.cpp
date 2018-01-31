/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include <cstdio>

#include "XML/XMLData.h"

using std::printf;


int main(void)
{
    CXMLData* pXML = CXMLData::CreateInstance();
    if (!pXML) {
        return -1;
    }

    CXMLData::XMLAttributes* pAttr1 = pXML->CreateXMLAttributes("Key1", "Value1");
    CXMLData::XMLAttributes* pAttr2 = pXML->CreateXMLAttributes("Key2", "Value2");
    CXMLData::XMLAttributes* pAttr3 = pXML->CreateXMLAttributes("Key3", "Value3");
    CXMLData::XMLAttributes* pAttr4 = pXML->CreateXMLAttributes("Key4", "Value4");
    CXMLData::XMLAttributes* pAttr5 = pXML->CreateXMLAttributes("Key5", "Value5");
    CXMLData::XMLAttributes* pAttr6 = pXML->CreateXMLAttributes("Key6", "Value6");

    if (!pAttr1 || !pAttr2 || !pAttr3 || !pAttr4 || !pAttr5 || !pAttr6) {
        return -1;
    }

    
    CXMLData::XMLElement* pElem1 = pXML->CreateXMLElement("Name1", "Text1", pAttr1);
    CXMLData::XMLElement* pElem2 = pXML->CreateXMLElement("Name2", "Text2", pAttr2);
    CXMLData::XMLElement* pElem3 = pXML->CreateXMLElement("Name3", "Text3", pAttr3);
    CXMLData::XMLElement* pElem4 = pXML->CreateXMLElement("Name4", "Text4", pAttr4);
    CXMLData::XMLElement* pElem5 = pXML->CreateXMLElement("Name5", "Text5", pAttr5);
    CXMLData::XMLElement* pElem6 = pXML->CreateXMLElement("Name6", "Text6", pAttr6);
    CXMLData::XMLElement* pElem7 = pXML->CreateXMLElement("Name6", "Text7", pAttr6);

    CTree<CXMLData::XMLElement>* pTree = pXML->GetXMLTree();
    assert(pTree);

    CTree<CXMLData::XMLElement>::CNode* pRoot = pTree->CreateRoot(pElem1);
    CTree<CXMLData::XMLElement>::CNode* pElem2Node = pTree->AddChild(pRoot, pElem2);
    CTree<CXMLData::XMLElement>::CNode* pElem3Node = pTree->AddChild(pRoot, pElem3);
    CTree<CXMLData::XMLElement>::CNode* pElem4Node = pTree->AddChild(pElem3Node, pElem4);
    CTree<CXMLData::XMLElement>::CNode* pElem5Node = pTree->AddChild(pElem3Node, pElem5);
    CTree<CXMLData::XMLElement>::CNode* pElem6Node = pTree->AddChild(pElem4Node, pElem6);
    CTree<CXMLData::XMLElement>::CNode* pElem7Node = pTree->AddChild(pElem3Node, pElem7);
    pTree->Dump();

    printf("----------------\n");
    printf("pElem1@Node: %p\n", pRoot);
    printf("pElem2@Node: %p\n", pElem2Node);
    printf("pElem3@Node: %p\n", pElem3Node);
    printf("pElem4@Node: %p\n", pElem4Node);
    printf("pElem5@Node: %p\n", pElem5Node);
    printf("pElem6@Node: %p\n", pElem6Node);
    printf("pElem7@Node: %p\n", pElem7Node);

    CXMLData::XMLElement checker("Name6");
    CTree<CXMLData::XMLElement>::FindResult* pResult = pTree->Find(checker);
    if (pResult) {
        for (size_t i = 0; i < pResult->Count; i++) {
            printf("Find the element at address: %p\n", pResult->pNodes[i]);
        }
        pResult->Release();
    }

    printf("--------------------------------\n");
    CTree<CXMLData::XMLElement>::CNode* pWho = pElem3Node->Find(checker);
    printf("Find the element at address: %p\n", pWho);

    printf("pTree Root count: %d\n", pRoot->Count());
    printf("pTree pElem3Node count: %d\n", pElem3Node->Count());
    printf("pTree pElem4Node count: %d\n", pElem4Node->Count());

    CXMLData::DestroyInstance(pXML);
    return 0;
}