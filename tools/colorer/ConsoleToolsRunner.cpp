
#include<stdio.h>
#include<stdlib.h>
#include<colorer/viewer/ConsoleTools.h>

/** Internal run action type */
enum { JT_NOTHING, JT_REGTEST, JT_PROFILE,
       JT_LIST_LOAD, JT_LIST_TYPES, JT_LIST_TYPE_NAMES,
       JT_VIEW, JT_GEN, JT_GEN_TOKENS, JT_FORWARD } jobType;
int profileLoops = 1;

/** Reads and parse command line */
void init(ConsoleTools &ct, int argc, char*argv[]){

  bool showBanner = true;

  for(int i = 1; i < argc; i++){
    if (argv[i][0] != '-'){
      ct.setInputFileName(DString(argv[i]));
      continue;
    }

    if (argv[i][1] == 'p'){
      jobType = JT_PROFILE;
      if (argv[i][2]){
        profileLoops = atoi(argv[i]+2);
      }
      continue;
    }
    if (argv[i][1] == 'r') { jobType = JT_REGTEST; continue; }
    if (argv[i][1] == 'f') { jobType = JT_FORWARD; continue; }
    if (argv[i][1] == 'v') { jobType = JT_VIEW; continue; }
    if (argv[i][1] == 'h' && argv[i][2] == 't') { jobType = JT_GEN_TOKENS; continue; }
    if (argv[i][1] == 'h') { jobType = JT_GEN; continue; }
    if (argv[i][1] == 'l' && argv[i][2] == 's' && (i+1 < argc || argv[i][3])){
      if (argv[i][3]){
        ct.setLinkSource(DString(argv[i]+3));
      }else{
        ct.setLinkSource(DString(argv[i+1]));
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'l' && argv[i][2] == 'n') { ct.addLineNumbers(true); continue; }
    if (argv[i][1] == 'l' && argv[i][2] == 'l') { jobType = JT_LIST_LOAD; continue; }
    if (argv[i][1] == 'l' && argv[i][2] == 't') { jobType = JT_LIST_TYPE_NAMES; continue; }
    if (argv[i][1] == 'l') { jobType = JT_LIST_TYPES; continue; }

    if (argv[i][1] == 'd' && argv[i][2] == 'c') { ct.setCopyrightHeader(false); showBanner = false; continue; }
    if (argv[i][1] == 'd' && argv[i][2] == 'b') { ct.setBomOutput(false); continue; }
    if (argv[i][1] == 'd' && argv[i][2] == 's') { ct.setHtmlEscaping(false); continue; }
    if (argv[i][1] == 'd' && argv[i][2] == 'h') { ct.setHtmlWrapping(false); continue; }

    if (argv[i][1] == 't' && (i+1 < argc || argv[i][2])){
      if (argv[i][2]){
        ct.setTypeDescription(DString(argv[i]+2));
      }else{
        ct.setTypeDescription(DString(argv[i+1]));
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'o' && (i+1 < argc || argv[i][2])){
      if (argv[i][2]){
        ct.setOutputFileName(DString(argv[i]+2));
      }else{
        ct.setOutputFileName(DString(argv[i+1]));
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'i' && (i+1 < argc || argv[i][2])){
      if (argv[i][2]){
        ct.setHRDName(DString(argv[i]+2));
      }else{
        ct.setHRDName(DString(argv[i+1]));
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'c' && (i+1 < argc || argv[i][2])){
      if (argv[i][2]){
        ct.setCatalogPath(DString(argv[i]+2));
      }else{
        ct.setCatalogPath(DString(argv[i+1]));
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'e' && argv[i][2] == 'i' && (i+1 < argc || argv[i][3])){
      if (argv[i][3]){
        ct.setInputEncoding(DString(argv[i]+3));
      }else{
        ct.setInputEncoding(DString(argv[i+1]));
        i++;
      }
      continue;
    }
    if (argv[i][1] == 'e' && argv[i][2] == 'o' && (i+1 < argc || argv[i][3])){
      if (argv[i][3]){
        ct.setOutputEncoding(DString(argv[i]+3));
      }else{
        ct.setOutputEncoding(DString(argv[i+1]));
        i++;
      }
      continue;
    }
    if (argv[i][1]) fprintf(stderr, "WARNING: unknown option '-%s'\n", argv[i]+1);
  }

  if (showBanner){
    fprintf(stderr, "\n%s\n", ParserFactory::getVersion());
    fprintf(stderr, "Copyright (c) 1999-2006 Igor Russkih <irusskih at gmail.com>\n\n");
  }

}

/** Prints usage. */
void printError(){
  fprintf(stderr,
       "Usage: colorer -(command) (parameters)  [<filename>]\n"
       " Commands:\n"
       "  -l         Lists all available languages\n"
       "  -lt        Lists all available languages (HRC types)\n"
       "  -ll        Lists and loads full HRC database\n"
       "  -r         RE tests\n"
       "  -h         Generates plain coloring from <filename> (uses 'rgb' hrd class)\n"
       "  -ht        Generates plain coloring from <filename> using tokens output\n"
       "  -v         Runs viewer on file <fname> (uses 'console' hrd class)\n"
       "  -p<n>      Runs parser in profile mode (if <n> specified, makes <n> loops)\n"
       "  -f         Forwards input file into output with specified encodings\n"
       " Parameters:\n"
       "  -c<path>   Uses specified 'catalog.xml' file\n"
       "  -i<name>   Loads specified hrd rules from catalog\n"
       "  -t<type>   Tries to use type <type> instead of type autodetection\n"
       "  -ls<name>  Use file <name> as input linking data source for href generation\n"
       "  -ei<name>  Use input file encoding <name>\n"
       "  -eo<name>  Use output stream encoding <name>, also viewer encoding in w9x\n"
       "  -o<name>   Use file <name> as output stream\n"
       "  -ln        Add line numbers into the colorized file\n"
       "  -db        Disable BOM(ZWNBSP) start symbol output in Unicode encodings\n"
       "  -dc        Disable information header in generator's output\n"
       "  -ds        Disable HTML symbol substitutions in generator's output\n"
       "  -dh        Disable HTML header and footer output\n"
  );
};

#include<common/MemoryChunks.h>

/** Creates ConsoleTools class instance and runs it.
*/
int main(int argc, char *argv[])
{
  ConsoleTools ct;  
  try{
    init(ct, argc, argv);
  }catch(Exception e){
    fprintf(stderr, e.getMessage()->getChars());
    return -1;
  };

  try{
    switch(jobType){
      case JT_REGTEST:
        ct.RETest();
        break;
      case JT_PROFILE:
        ct.profile(profileLoops);
        break;
      case JT_LIST_LOAD:
        ct.listTypes(true, false);
        break;
      case JT_LIST_TYPES:
        ct.listTypes(false, false);
        break;
      case JT_LIST_TYPE_NAMES:
        ct.listTypes(false, true);
        break;
      case JT_VIEW:
        ct.viewFile();
        break;
      case JT_GEN:
        ct.genOutput();
        break;
      case JT_GEN_TOKENS:
        ct.genTokenOutput();
        break;
      case JT_FORWARD:
        ct.forward();
        break;
      default:
        printError();
        break;
    };
  }catch(Exception e){
    fprintf(stderr, e.getMessage()->getChars());
    return -1;
  };

//  printf("Memory Profiling: total_req:%d, new_calls:%d, free_calls:%d\n", get_total_req(), get_new_calls(), get_free_calls());

  return 0;
};

/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Colorer Library
 *
 * The Initial Developer of the Original Code is
 * Cail Lomecb <cail@nm.ru>.
 * Portions created by the Initial Developer are Copyright (C) 1999-2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */