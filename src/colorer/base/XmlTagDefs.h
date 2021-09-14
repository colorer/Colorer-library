#ifndef COLORER_XMLTAGDEFS_H
#define COLORER_XMLTAGDEFS_H

/* catalog.xml

<catalog>
  <hrc-sets>
    <location link=""/>
  </hrc-sets>
  <hrd-sets>
    <hrd class="" name="" description="">
      <location link=""/>
    </hrd>
  </hrd-sets>
</catalog>
*/

const char16_t catTagCatalog[] = u"catalog\0";
const char16_t catTagHrcSets[] = u"hrc-sets\0";
const char16_t catTagLocation[] = u"location\0";
const char16_t catLocationAttrLink[] = u"link\0";
const char16_t catTagHrdSets[] = u"hrd-sets\0";
const char16_t catTagHrd[] = u"hrd\0";
const char16_t catHrdAttrClass[] = u"class\0";
const char16_t catHrdAttrName[] = u"name\0";
const char16_t catHrdAttrDescription[] = u"description\0";

/* hrc file

<hrc version="">
  <annotation/>
  <prototype name="" group="" description="">
    <annotation/>
    <location link=""/>
    <filename weight=""></filename>
    <firstline weight=""></firstline>
    <parameters>
      <param name="" value="" description=""/>
    </parameters>
  </prototype>
  <package name="" group="" description="">
    <annotation/>
    <location link=""/>
  </package>
  <type name="">
    <annotation/>
    <import type=""/>
    <region name="" parent="" description=""/>
    <entity name="" value=""/>
    <scheme name="" if="" unless="">
      <annotation/>
      <keywords ignorecase="" region="" priority="" worddiv="">
        <word name="" region=""/>
        <symb name="" region=""/>
      </keywords>
      <regexp match="" region="" priority=""/>
      <block start="" end="" scheme="" priority="" content-priority="" inner-region="" region=""/>
      <inherit scheme="">
        <virtual scheme="" subst-scheme=""/>
      </inherit>
    </scheme>
  </type>
</hrc>
*/

const char16_t hrcTagHrc[] = u"hrc\0";
const char16_t hrcHrcAttrVersion[] = u"version\0";
const char16_t hrcTagAnnotation[] = u"annotation\0";
const char16_t hrcTagPrototype[] = u"prototype\0";
const char16_t hrcPrototypeAttrName[] = u"name\0";
const char16_t hrcPrototypeAttrDescription[] = u"description\0";
const char16_t hrcPrototypeAttrGroup[] = u"group\0";
const char16_t hrcTagLocation[] = u"location\0";
const char16_t hrcLocationAttrLink[] = u"link\0";
const char16_t hrcTagFilename[] = u"filename\0";
const char16_t hrcFilenameAttrWeight[] = u"weight\0";
const char16_t hrcTagFirstline[] = u"firstline\0";
const char16_t hrcFirstlineAttrWeight[] = u"weight\0";
const char16_t hrcTagParametrs[] = u"parameters\0";
const char16_t hrcTagParam[] = u"param\0";
const char16_t hrcParamAttrName[] = u"name\0";
const char16_t hrcParamAttrValue[] = u"value\0";
const char16_t hrcParamAttrDescription[] = u"description\0";
const char16_t hrcTagPackage[] = u"package\0";
const char16_t hrcPackageAttrName[] = u"name\0";
const char16_t hrcPackageAttrDescription[] = u"description\0";
const char16_t hrcPackageAttrGroup[] = u"group\0";
const char16_t hrcTagType[] = u"type\0";
const char16_t hrcTypeAttrName[] = u"name\0";
const char16_t hrcTagImport[] = u"import\0";
const char16_t hrcImportAttrType[] = u"type\0";
const char16_t hrcTagRegion[] = u"region\0";
const char16_t hrcRegionAttrName[] = u"name\0";
const char16_t hrcRegionAttrParent[] = u"parent\0";
const char16_t hrcRegionAttrDescription[] = u"description\0";
const char16_t hrcTagEntity[] = u"entity\0";
const char16_t hrcEntityAttrName[] = u"name\0";
const char16_t hrcEntityAttrValue[] = u"value\0";
const char16_t hrcTagScheme[] = u"scheme\0";
const char16_t hrcSchemeAttrName[] = u"name\0";
const char16_t hrcSchemeAttrIf[] = u"if\0";
const char16_t hrcSchemeAttrUnless[] = u"unless\0";
const char16_t hrcTagKeywords[] = u"keywords\0";
const char16_t hrcKeywordsAttrIgnorecase[] = u"ignorecase\0";
const char16_t hrcKeywordsAttrPriority[] = u"priority\0";
const char16_t hrcKeywordsAttrWorddiv[] = u"worddiv\0";
const char16_t hrcKeywordsAttrRegion[] = u"region\0";
const char16_t hrcTagWord[] = u"word\0";
const char16_t hrcWordAttrName[] = u"name\0";
const char16_t hrcWordAttrRegion[] = u"region\0";
const char16_t hrcTagSymb[] = u"symb\0";
const char16_t hrcSymbAttrName[] = u"name\0";
const char16_t hrcSymbAttrRegion[] = u"region\0";
const char16_t hrcTagRegexp[] = u"regexp\0";
const char16_t hrcRegexpAttrMatch[] = u"match\0";
const char16_t hrcRegexpAttrPriority[] = u"priority\0";
const char16_t hrcRegexpAttrRegion[] = u"region\0";
const char16_t hrcTagBlock[] = u"block\0";
const char16_t hrcBlockAttrStart[] = u"start\0";
const char16_t hrcBlockAttrEnd[] = u"end\0";
const char16_t hrcBlockAttrScheme[] = u"scheme\0";
const char16_t hrcBlockAttrPriority[] = u"priority\0";
const char16_t hrcBlockAttrContentPriority[] = u"content-priority\0";
const char16_t hrcBlockAttrInnerRegion[] = u"inner-region\0";
const char16_t hrcBlockAttrMatch[] = u"match\0";
const char16_t hrcTagInherit[] = u"inherit\0";
const char16_t hrcInheritAttrScheme[] = u"scheme\0";
const char16_t hrcTagVirtual[] = u"virtual\0";
const char16_t hrcVirtualAttrScheme[] = u"scheme\0";
const char16_t hrcVirtualAttrSubstScheme[] = u"subst-scheme\0";

/* hrd file
<hrd>
  <assign name="" fore="" back="" style=""/>
  <assign name="" stext="" etext="" sback="" eback=""/>
</hrc>
*/
const char16_t hrdTagHrd[] = u"hrd\0";
const char16_t hrdTagAssign[] = u"assign\0";
const char16_t hrdAssignAttrName[] = u"name\0";
const char16_t hrdAssignAttrFore[] = u"fore\0";
const char16_t hrdAssignAttrBack[] = u"back\0";
const char16_t hrdAssignAttrStyle[] = u"style\0";
const char16_t hrdAssignAttrSBack[] = u"sback\0";
const char16_t hrdAssignAttrEBack[] = u"eback\0";
const char16_t hrdAssignAttrSText[] = u"stext\0";
const char16_t hrdAssignAttrEText[] = u"etext\0";

#endif  // COLORER_XMLTAGDEFS_H
