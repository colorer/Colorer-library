#ifndef _COLORER_XMLTAGDEFS_H_
#define _COLORER_XMLTAGDEFS_H_

#include <xercesc/util/XMLUniDefs.hpp>
using namespace xercesc;

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

const XMLCh hrcTagHrc[] = {chLatin_h, chLatin_r, chLatin_c, chNull};
const XMLCh hrcHrcAttrVersion[] = {chLatin_v, chLatin_e, chLatin_r, chLatin_s, chLatin_i, chLatin_o, chLatin_n, chNull};
const XMLCh hrcTagAnnotation[] = {chLatin_a, chLatin_n, chLatin_n, chLatin_o, chLatin_t, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull};
const XMLCh hrcTagPrototype[] = {chLatin_p, chLatin_r, chLatin_o, chLatin_t, chLatin_o, chLatin_t, chLatin_y, chLatin_p, chLatin_e, chNull};
const XMLCh hrcPrototypeAttrName[] = {chLatin_n, chLatin_a, chLatin_m, chLatin_e, chNull};
const XMLCh hrcPrototypeAttrDescription[] = {chLatin_d, chLatin_e, chLatin_s, chLatin_c, chLatin_r, chLatin_i, chLatin_p, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull};
const XMLCh hrcPrototypeAttrGroup[] = {chLatin_g, chLatin_r, chLatin_o, chLatin_u, chLatin_p, chNull};
const XMLCh hrcTagLocation[] = {chLatin_l, chLatin_o, chLatin_c, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull};
const XMLCh hrcLocationAttrLink[] = {chLatin_l, chLatin_i, chLatin_n, chLatin_k, chNull};
const XMLCh hrcTagFilename[] = {chLatin_f, chLatin_i, chLatin_l, chLatin_e, chLatin_n, chLatin_a, chLatin_m, chLatin_e, chNull};
const XMLCh hrcFilenameAttrWeight[] = {chLatin_w, chLatin_e, chLatin_i, chLatin_g, chLatin_h, chLatin_t, chNull}; 
const XMLCh hrcTagFirstline[] = {chLatin_f, chLatin_i, chLatin_r, chLatin_s, chLatin_t, chLatin_l, chLatin_i, chLatin_n, chLatin_e, chNull};
const XMLCh hrcFirstlineAttrWeight[] = {chLatin_w, chLatin_e, chLatin_i, chLatin_g, chLatin_h, chLatin_t, chNull}; 
const XMLCh hrcTagParametrs[] = {chLatin_p, chLatin_a, chLatin_r, chLatin_a, chLatin_m, chLatin_e, chLatin_t, chLatin_e, chLatin_r, chLatin_s, chNull};
const XMLCh hrcTagParam[] = {chLatin_p, chLatin_a, chLatin_r, chLatin_a, chLatin_m, chNull};
const XMLCh hrcParamAttrName[] = {chLatin_n, chLatin_a, chLatin_m, chLatin_e, chNull};
const XMLCh hrcParamAttrValue[] = {chLatin_v, chLatin_a, chLatin_l, chLatin_u, chLatin_e, chNull};
const XMLCh hrcParamAttrDescription[] = {chLatin_d, chLatin_e, chLatin_s, chLatin_c, chLatin_r, chLatin_i, chLatin_p, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull};
const XMLCh hrcTagPackage[] = {chLatin_p, chLatin_a, chLatin_c, chLatin_k, chLatin_a, chLatin_g, chLatin_e, chNull};
const XMLCh hrcPackageAttrName[] = {chLatin_n, chLatin_a, chLatin_m, chLatin_e, chNull};
const XMLCh hrcPackageAttrDescription[] = {chLatin_d, chLatin_e, chLatin_s, chLatin_c, chLatin_r, chLatin_i, chLatin_p, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull};
const XMLCh hrcPackageAttrGroup[] = {chLatin_g, chLatin_r, chLatin_o, chLatin_u, chLatin_p, chNull};
const XMLCh hrcTagType[] = {chLatin_t, chLatin_y, chLatin_p, chLatin_e, chNull};
const XMLCh hrcTypeAttrName[] = {chLatin_n, chLatin_a, chLatin_m, chLatin_e, chNull};
const XMLCh hrcTagImport[] = {chLatin_i, chLatin_m, chLatin_p, chLatin_o, chLatin_r, chLatin_t, chNull};
const XMLCh hrcImportAttrType[] = {chLatin_t, chLatin_y, chLatin_p, chLatin_e, chNull};
const XMLCh hrcTagRegion[] = {chLatin_r, chLatin_e, chLatin_g, chLatin_i, chLatin_o, chLatin_n, chNull}; 
const XMLCh hrcRegionAttrName[] = {chLatin_n, chLatin_a, chLatin_m, chLatin_e, chNull};
const XMLCh hrcRegionAttrParent[] = {chLatin_p, chLatin_a, chLatin_r, chLatin_e, chLatin_n, chLatin_t, chNull};
const XMLCh hrcRegionAttrDescription[] = {chLatin_d, chLatin_e, chLatin_s, chLatin_c, chLatin_r, chLatin_i, chLatin_p, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull};
const XMLCh hrcTagEntity[] = {chLatin_e, chLatin_n, chLatin_t, chLatin_i, chLatin_t, chLatin_y, chNull};
const XMLCh hrcEntityAttrName[] = {chLatin_n, chLatin_a, chLatin_m, chLatin_e, chNull};
const XMLCh hrcEntityAttrValue[] = {chLatin_v, chLatin_a, chLatin_l, chLatin_u, chLatin_e, chNull};
const XMLCh hrcTagScheme[] = {chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_e, chNull};
const XMLCh hrcSchemeAttrName[] = {chLatin_n, chLatin_a, chLatin_m, chLatin_e, chNull};
const XMLCh hrcSchemeAttrIf[] = {chLatin_i, chLatin_f, chNull};
const XMLCh hrcSchemeAttrUnless[] = {chLatin_u, chLatin_n, chLatin_l, chLatin_e, chLatin_s, chLatin_s, chNull};
const XMLCh hrcTagKeywords[] = {chLatin_k, chLatin_e, chLatin_y, chLatin_w, chLatin_o, chLatin_r, chLatin_d, chLatin_s, chNull};
const XMLCh hrcKeywordsAttrIgnorecase[] = {chLatin_i, chLatin_g, chLatin_n, chLatin_o, chLatin_r, chLatin_e, chLatin_c, chLatin_a, chLatin_s, chLatin_e, chNull};
const XMLCh hrcKeywordsAttrPriority[] = {chLatin_p, chLatin_r, chLatin_i, chLatin_o, chLatin_r, chLatin_i, chLatin_t, chLatin_y, chNull};
const XMLCh hrcKeywordsAttrWorddiv[] = {chLatin_w, chLatin_o, chLatin_r, chLatin_d, chLatin_d, chLatin_i, chLatin_v, chNull};
const XMLCh hrcKeywordsAttrRegion[] = {chLatin_r, chLatin_e, chLatin_g, chLatin_i, chLatin_o, chLatin_n, chNull};
const XMLCh hrcTagWord[] = {chLatin_w, chLatin_o, chLatin_r, chLatin_d, chNull};
const XMLCh hrcWordAttrName[]= {chLatin_n, chLatin_a, chLatin_m, chLatin_e, chNull};
const XMLCh hrcWordAttrRegion[] = {chLatin_r, chLatin_e, chLatin_g, chLatin_i, chLatin_o, chLatin_n, chNull};
const XMLCh hrcTagSymb[] = {chLatin_s, chLatin_y, chLatin_m, chLatin_b, chNull};
const XMLCh hrcSymbAttrName[]= {chLatin_n, chLatin_a, chLatin_m, chLatin_e, chNull};
const XMLCh hrcSymbAttrRegion[] = {chLatin_r, chLatin_e, chLatin_g, chLatin_i, chLatin_o, chLatin_n, chNull};
const XMLCh hrcTagRegexp[] = {chLatin_r, chLatin_e, chLatin_g, chLatin_e, chLatin_x, chLatin_p, chNull};
const XMLCh hrcRegexpAttrMatch[] = {chLatin_m, chLatin_a, chLatin_t, chLatin_c, chLatin_h, chNull};
const XMLCh hrcRegexpAttrPriority[] = {chLatin_p, chLatin_r, chLatin_i, chLatin_o, chLatin_r, chLatin_i, chLatin_t, chLatin_y, chNull};
const XMLCh hrcRegexpAttrRegion[] = {chLatin_r, chLatin_e, chLatin_g, chLatin_i, chLatin_o, chLatin_n, chNull};
const XMLCh hrcTagBlock[] = {chLatin_b, chLatin_l, chLatin_o, chLatin_c, chLatin_k, chNull};
const XMLCh hrcBlockAttrStart[] = {chLatin_s, chLatin_t, chLatin_a, chLatin_r, chLatin_t, chNull};
const XMLCh hrcBlockAttrEnd[] = {chLatin_e, chLatin_n, chLatin_d, chNull};
const XMLCh hrcBlockAttrScheme[] = {chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_e, chNull};
const XMLCh hrcBlockAttrPriority[] = {chLatin_p, chLatin_r, chLatin_i, chLatin_o, chLatin_r, chLatin_i, chLatin_t, chLatin_y, chNull};
const XMLCh hrcBlockAttrContentPriority[] = {chLatin_c, chLatin_o, chLatin_n, chLatin_t, chLatin_e, chLatin_n, chLatin_t, chDash, chLatin_p, chLatin_r, chLatin_i, chLatin_o, chLatin_r, chLatin_i, chLatin_t, chLatin_y, chNull};
const XMLCh hrcBlockAttrInnerRegion[] = {chLatin_i, chLatin_n, chLatin_n, chLatin_e, chLatin_r, chDash, chLatin_r, chLatin_e, chLatin_g, chLatin_i, chLatin_o, chLatin_n, chNull};
const XMLCh hrcBlockAttrMatch[] = {chLatin_m, chLatin_a, chLatin_t, chLatin_c, chLatin_h, chNull};
const XMLCh hrcTagInherit[] = {chLatin_i, chLatin_n, chLatin_h, chLatin_e, chLatin_r, chLatin_i, chLatin_t, chNull};
const XMLCh hrcInheritAttrScheme[] = {chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_e, chNull};
const XMLCh hrcTagVirtual[] = {chLatin_v, chLatin_i, chLatin_r, chLatin_t, chLatin_u, chLatin_a, chLatin_l, chNull};
const XMLCh hrcVirtualAttrScheme[] = {chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_e, chNull};
const XMLCh hrcVirtualAttrSubstScheme[] = {chLatin_s, chLatin_u, chLatin_b, chLatin_s, chLatin_t, chDash, chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_e, chNull};


#endif // _COLORER_XMLTAGDEFS_H_