# PROJECT:        Deltarune MD
# FILE:           tools/resgen.py
# AUTHOR:         RatcheT2497
# CREATION:       23/09/21
# MODIFIED:       25/09/21
# DESCRIPTION:    Resource file preprocessor.
#                 Usage: resgen.py [src] [out]
#                  #! - begins preprocessor command
#                  #@ - preprocessor data definition
#                  Everything else gets copied verbatim to the output file
# CHANGELOG:      (25/09/21) Added this file header and some comments. -R#

# this script is a bit of a mess, i'm not gonna lie lol
# seems to work though
import sys
import struct
import os
def tokenize(line):
    i = 0
    string_char = None
    tokens = []
    temp = ''
    while i < len(line):
        if string_char is None:
            if line[i] == '"' or line[i] == '\'':
                string_char = line[i]
            elif line[i] in ' \t':
                if temp:
                    tokens.append(temp)
                    temp = ''
            else:
                temp += line[i]
        else:
            if line[i] == string_char:
                string_char = None
                tokens.append(temp)
                temp = ''
            else:
                temp += line[i]
        i += 1
    if temp:
        tokens.append(temp)
    return tokens

inpath = sys.argv[1]
outpath = sys.argv[2]
lines = None
with open(inpath, "r") as file:
    lines = file.readlines()
if lines is None:
    raise Exception('readlines error')

colmap = False
colmap_name = ''
colmap_path = ''
colmap_width = 0
colmap_height = 0
colmap_array = []
colmap_dict = {
    '.': 0,
    '#': 1,
    '~': 15
}
output_lines = []
for line in lines:
    line = line.strip()
    if not line:
        continue
    if line[0] == '#':
        if len(line) > 1:
            if line[1] == '!':
                tokens = tokenize(line)
                if len(tokens) == 0:
                    continue
                # preprocessor command
                if tokens[0] == '#!COLMAP':
                    colmap_name = tokens[1]
                    colmap_path = tokens[2]
                    colmap_width = int(tokens[3], 0)
                    colmap_height = int(tokens[4], 0)
                    colmap = True
                elif tokens[0] == '#!ENDCOL':
                    output_lines.append(f'BIN {colmap_name} {colmap_path} 2 2 0 NONE\n')
                    with open(os.path.join('res', colmap_path), 'wb') as of:
                        for val in colmap_array:
                            of.write(struct.pack('B', val))
                    colmap_array = []
                    colmap_width = 0
                    colmap_height = 0
                    colmap_name = ''
                    colmap_path = ''
                    colmap = False
                pass
            elif line[1] == '@':
                # preprocessor data
                # TODO: make this more extensible
                # NOTE: currently limited to collision map data
                if not colmap:
                    continue
                line = line.split(' ')[1]
                linebuf = []
                temp = 0
                for line_i in range(0, len(line) // 2):
                    left = line[line_i*2]
                    right = line[1+line_i*2]
                    temp = (colmap_dict[left] << 4) | colmap_dict[right]
                    linebuf.append(temp)
                for val in linebuf:
                    colmap_array.append(val)
            else:
                output_lines.append(line + '\n')
        continue
    else:
        output_lines.append(line + '\n')

with open(outpath, 'w') as of:
    of.writelines(output_lines)

print('resgen finish')