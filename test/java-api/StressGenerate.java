
import net.sf.colorer.*;
import net.sf.colorer.editor.*;
import net.sf.colorer.impl.*;
import net.sf.colorer.viewer.*;

import java.io.IOException;
import java.io.Writer;

import java.io.*;
import java.util.*;

/**
 * Simple test routine, used to test Colorer library working.
 *
 * @author irusskih
 *
 */
class StressGenerate {


    static RegionHandler reg = new RegionHandler(){
    
        public void addRegion(int lno, String line, int sx, int ex, Region region)
        {
        }

        public void clearLine(int lno, String line)
        {
        }

        public void endParsing(int lno)
        {
        }

        public void enterScheme(int lno, String line, int sx, int ex, Region region, String scheme)
        {
            System.out.println("enterScheme:"+scheme);
        }

        public void leaveScheme(int lno, String line, int sx, int ex, Region region, String scheme)
        {
        }

        public void startParsing(int lno)
        {
        }
    
    };
    
    public static void main(String[] arr) {

        try {
            ParserFactory pf = new ParserFactory();

            for(int l = 0; l < 1000; l++)
            {
                ReaderLineSource lineSource = new ReaderLineSource(new FileReader("StressGenerate.java"));

                BaseEditor be = new BaseEditorNative(pf, lineSource);
                
                be.setRegionCompact(true);
                //be.setRegionMapper("rgb", "default");
                be.setRegionMapper("rgb", "default");

                be.lineCountEvent(lineSource.getLineCount());
                be.visibleTextEvent(0, lineSource.getLineCount());
                be.chooseFileType("asdf.java");

                be.addRegionHandler(reg, null);
                
                Writer commonWriter = new PrintWriter(System.out);
                Writer escapedWriter = commonWriter;
                
                for (int idx = 0; idx < lineSource.getLineCount(); idx++)
                {
                  if (be.getLineRegions(idx) != null && be.getLineRegions(idx).length > 0)
                    commonWriter.write("scheme:"+be.getLineRegions(idx)[0].scheme+"\n");
                  ParsedLineWriter.htmlRGBWrite(commonWriter, escapedWriter, lineSource.getLine(idx), be.getLineRegions(idx));
                  commonWriter.write("\n");
                }            
                commonWriter.flush();
                be.dispose();
            }
        }catch(Exception e){
            e.printStackTrace();
        }
    }

}
