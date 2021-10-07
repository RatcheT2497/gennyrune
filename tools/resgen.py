# PROJECT:        Deltarune MD
# FILE:           tools/resgen.py
# AUTHOR:         RatcheT2497
# CREATION:       23/09/21
# MODIFIED:       06/10/21
# DESCRIPTION:    Resource file preprocessor.
#                 Usage: resgen.py [src] [out]
#                  #! - begins preprocessor command
#                  #@ - preprocessor data definition
#                  Everything else gets copied verbatim to the output file
# CHANGELOG:      (25/09/21) Added this file header and some comments. -R#
#                 (06/10/21) Rewrote the generator script entirely. -R#

import sys
import struct
import os

class Generator:
    """
    Base asset generator class.
    """
    def __init__(self, line, tokens, is_single_line):
        pass
    def add_line_data(self, line, tokens):
        pass
    def finalize(self):
        pass

COLMAP_DICT = {
    '.': 0,
    '#': 1,
    '~': 15
}
class CollisionMapGenerator(Generator):
    """
        #!COLMAP <name> <path> <width> <height>
        #@ ..... - empty
        #@ ##### - solid
        #@ ~~~~~ - trigger
        #!END
    """
    def __init__(self, line, tokens, is_single_line):
        self.name = tokens[1]
        self.path = tokens[2]
        self.width = int(tokens[3], 0)
        self.height = int(tokens[4], 0)
        if (self.width % 2) != 0:
            raise Exception(f'CollisionMapGenerator init: invalid map width ({self.width}): not a power of 2')
        self.array = []

    def add_line_data(self, line, tokens):
        # not using the passed tokens and instead just splitting the line on spaces
        split_line = line.strip().split(' ')
        if len(split_line) != 2:
            raise Exception(f'CollisionMapGenerator add_line_data: invalid number of data arguments ({len(split_line)} instead of 2)')
        
        # index 0 should be the data definition symbol, 1 should be the data
        data_line = split_line[1]
        if len(data_line) != self.width:
            raise Exception(f'CollisionMapGenerator add_line_data: invalid map width ({len(data_line)} instead of {self.width})')

        for line_i in range(0, len(data_line) // 2):
            left = data_line[line_i * 2]
            right = data_line[1 + line_i * 2]

            # pack two tiles per byte
            value = (COLMAP_DICT[left] << 4) | COLMAP_DICT[right]
            self.array.append(value)


    def finalize(self):
        # TODO: make this path modifiable
        with open(os.path.join('res', self.path), 'wb') as of:
            for val in self.array:
                of.write(struct.pack('B', val))

        return [f'BIN {self.name} \"{self.path}\" 2 2 0 NONE\n']

class LevelGenerator(Generator):
    """
        #!LEVEL <name> <path>
    """
    def __init__(self, line, tokens, is_single_line):
        self.name = tokens[1]
        self.path = tokens[2]

    def finalize(self):
        return [
            f'# LEVEL: {self.name}\n',
            f'TILESET tls_{self.name} \"{self.path}\" NONE ALL\n',
            f'PALETTE pal_{self.name} \"{self.path}\"\n',
            f'MAP map_{self.name} \"{self.path}\" tls_{self.name}\n',
        ]

    def add_line_data(self, line, tokens):
        pass

GENERATOR_LUT = {
    '#!COLMAP': (CollisionMapGenerator, False),
    '#!LEVEL': (LevelGenerator, True)
}
class ResFileParser:
    def _read_file_lines(self, path):
        with open(path, 'r') as file:
            self.lines = file.readlines()
        for i in range(0, len(self.lines)):
            self.lines[i] = self.lines[i].strip()
    def _tokenize_line(self, line):
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
    def __init__(self, path):
        self.lines = None
        self.active_generator = None
        self.path = path
    
    def parse(self):
        output_lines = []
        self._read_file_lines(self.path)

        line_n = 1
        for line in self.lines:
            if not line:
                continue
            tokens = self._tokenize_line(line)
            if len(line) > 2:
                if line[0] == '#':
                    if line[1] == '!':
                        # command
                        if tokens[0] == '#!END':
                            if self.active_generator is None:
                                raise Exception('line {line_n}: invalid #!END without active generator')
                            output_lines += self.active_generator.finalize()
                            self.active_generator = None
                        else:
                            if not (self.active_generator is None):
                                raise Exception('line {line_n}: invalid command while generator active')
                            try:
                                (generator_class, is_single_line) = GENERATOR_LUT[tokens[0]]
                                self.active_generator = generator_class(line, tokens, is_single_line)
                                if is_single_line:
                                    output_lines += self.active_generator.finalize()
                                    self.active_generator = None
                            except Exception as e:
                                raise Exception(f'line {line_n}: {e}')
                    elif line[1] == '@':
                        # data definition
                        if self.active_generator is None:
                            raise Exception(f'line {line_n}: invalid inline data definition without active generator')
                        try:
                            self.active_generator.add_line_data(line, tokens)
                        except Exception as e:
                            raise Exception(f'line {line_n}: {e}')
                    else:
                        # pure comment - copy
                        output_lines.append(line + '\n')
                else:
                    # final definition - copy without preprocessing
                    output_lines.append(line + '\n')
            else:
                # no commands smaller than 2 characters lol
                if line[0] != '#':
                    raise Exception(f'line {line_n}: garbage character')
            line_n += 1
        return output_lines
                
if __name__ == '__main__':
    inpath = sys.argv[1]
    outpath = sys.argv[2]
    # read input file
    lines = None
    with open(inpath, "r") as file:
        lines = file.readlines()
    if lines is None:
        raise Exception('readlines error')

    parser = ResFileParser(inpath)
    output_lines = parser.parse()

    # write out new file
    with open(outpath, 'w') as of:
        of.writelines(output_lines)
    print('resgen finish')