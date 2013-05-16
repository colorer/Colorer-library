
import net.sf.colorer.*;
import net.sf.colorer.editor.*;
import net.sf.colorer.impl.*;

import java.util.*;

/**
 * Simple test routine, used to test Colorer library working.
 *
 * @author irusskih
 *
 */
class Test {

    public static void main(String[] arr) {

        /*
         * Initial parser factory is created from one of the predefined
         * locations It is used to created low-level parse objects and to
         * associate them with data files (HRC, HRD)
         */
        ParserFactory pf = new ParserFactory();

        /*
         * Trying to enumerate all available HRD styles from "rgb" coloring
         * class
         */
        for (Enumeration e = pf.enumerateHRDInstances("rgb"); e
                .hasMoreElements();) {
            String name = (String) e.nextElement();
            System.out.println(name + " - " + pf.getHRDescription("rgb", name));
        }

        /*
         * Creating BaseEditor - common object, which encapsulates internal
         * parsing algorithms and works as a facade for a couple of internal
         * objects and relations
         */
        BaseEditor be = new BaseEditorNative(pf, new LineSource() {
            // Using simple stub as an input source
            public String getLine(int no) {
                return null;
            };
        });

        /*
         * Enumerating all language types, available in the current HRC
         * database, compiling and loading them into our editor object
         */
        for (Enumeration e = pf.getHRCParser().enumerateFileTypes(); e.hasMoreElements();) {
            FileType type = (FileType) e.nextElement();
            System.out.println("loading: " + type.getDescription());
            String[] pars = type.getParameters();
            for (int i = 0; i < pars.length; i++) {
                String pname = pars[i];
                System.out.println("  param " + pname + " = "
                        + type.getParameterValue(pname));
            }
            be.setFileType(type);

        }
        ;

    };

}
