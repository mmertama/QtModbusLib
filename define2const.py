import re
import sys
import os

inf = sys.argv[1]
outf = sys.argv[2]

lines = list()

with open (outf) as f:
    for l in f:
        m = re.match(r"\s*#GENERATE", l)
        if(m):
            with open (inf) as f:
                for l in f:
                    m = re.match(r"\s*#define\s+(MODBUS_[a-zA-Z0-9_]+)\s+(0x[0-9a-fA-F]+|\d+)", l)
                    if(m):
                        lines.append("const int " + m.group(1) + " = " + m.group(2) + ";\n");
        else:
            lines.append(l)

namel = os.path.splitext(outf)[0] + ".h"

    
with open(namel, 'w') as f:
    f.writelines(lines)
                        

        
            



