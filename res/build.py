
import sys
import os
import json
import math

# utility functions for file reading
def readall(path):
    try:
        strn = ""
        with open(path, 'r') as f:
            strn = f.read()
        return strn
    except:
        return ''
def readjson(path):
    return json.loads(readall(path))
def updatefile(path, contents):
    if readall(path) != contents:
        print(f'writing {path}...')
        with open(path, "w") as f:
            f.write(contents)

# method for automatically generating a C array
def generate_array(type, name, fmt, data, line_width):
    s = f'{type} {name}[] = {{ '
    if line_width > 0:
        s += '\n'
    
    index = 0
    for val in data:
        if line_width > 0 and ((index % line_width) == 0):
            s += '    '
        s += f'{fmt.format(val)}, '
        if line_width > 0 and (index < len(data)-1) and ((index % line_width) == (line_width-1)):
            s += '\n'

        index += 1
    s += '\n};\n'
    return s

def c_header_guard(path, body):
    guard = '_' + path.upper().replace('.', '_').replace('\\', '_').replace('/', '_') + '_'

    return f'#ifndef {guard}\n#define {guard}\n{body}\n#endif\n'
def c_header_file(path, includes, body):
    s = ''
    for v in includes:
        s += f'#include "{v}"\n'
    return c_header_guard(path, s + body)
def generator_map(asset, basedirs):
    resdir = basedirs[2]

    name = asset['Name']
    path = asset['Path']

    #mapjsonpath = os.path.splitext(os.path.normpath(os.path.join(resdir, path)))[0] + ".json"
    #mapjson = readjson(mapjsonpath)

    #mapwidth = mapjson['width']
    #mapheight = mapjson['height']

    #actorpalette = 'NULL'
    #colarray = []
    #actors = []
    #actorids = {}
    #if mapwidth % 2 == 0:
    #    for layer in mapjson['layers']:
    #        if layer['name'] == 'Collision':
    #            # generate collision data (each tile: 4 bits)
    #            for y in range(0, mapheight):
    #                for x in range(0, mapwidth // 2):
    #                    left = (layer['data'][x*2 + y * mapwidth] & 0xF)
    #                    right = layer['data'][x*2 + 1 + y * mapwidth] & 0xF
    #                    val = (left << 4) | right
    #                    colarray.append(val)
    #        elif layer['name'] == 'Actors':
    #            actorpalette = '&pal_' + layer_get_property(layer, 'Palette')
    #            for obj in layer['objects']:
    #                flags = 1 # actor is enabled
    #                
    #                x = obj['x']
    #                y = obj['y']
    #                type = int(obj['type'], 0)
    #                sprdef = obj['name']
    #                id = layer_get_property(obj, 'Index')
    #                anim = layer_get_property(obj, 'Animation')
    #                depth = layer_get_property(obj, 'Depth')
    #                
    #                if obj['visible']:
    #                    flags |= 0b00000010
    #                
    #                actorids[id] = len(actors)
    #                actors.append((flags, type, x, y, depth, anim, sprdef))
    #else:
    #    raise Exception(f'map {name} width indivisible by 2')
    
    #realactors = []
    #for i in actorids:
    #    realactors.append(actors[i])
    # generate individual resources (actors and collision data)
    data_local = []
    #if len(realactors) > 0:
    #    data_local.append(('const MapActorDef_t', f'actors_{name}', '{{ 0x{0[0]:04X}, 0x{0[1]:04X}, {0[2]}, {0[3]}, {0[4]}, {0[5]}, &{0[6]} }}', realactors, 1))
    #data_local.append(('const u8', f'col_{name}', '0x{0:02X}', colarray, mapwidth//2))
    
    # generate the container map array data for this map
    #actorsname = 'NULL'
    #if len(actors) > 0:
    #    actorsname = f'actors_{name}'
    #data_global = [
    #    ('content_maps', [[0, mapwidth, mapheight, f'&pal_{name}', f'&tls_{name}', f'&map_{name}', f'col_{name}', actorpalette, len(actors), actorsname]] )
    #]
    data_global = [
        #('content_maps', [[0, mapwidth, mapheight, f'&pal_{name}', f'&tls_{name}', f'&map_{name}', f'col_{name}', actorpalette, len(actors), actorsname]] )
    ]
    return data_local, data_global
def saver_single_text(filebasedirs, filenames, data, saveconfig):
    finalpath = os.path.join(filebasedirs[0], filenames[0])
    s = ''
    for v in data[0]:
        for k in v:
            s += k + '\n'
    updatefile(finalpath, s)
def saver_c(filebasedirs, filenames, data, saveconfig):
    headerpath = os.path.join(filebasedirs[0], filenames[0])
    sourcepath = os.path.join(filebasedirs[1], filenames[1])
    
    localdata = data[0]
    globaldata = data[1]

    contents = saveconfig['contents']

    data = {}
    for d in globaldata:
        name = d[0]
        values = d[1]
        if not name in data:
            data[name] = []
        data[name] += values

    localstr = ''
    for group in localdata:
        for val in group:
            string = generate_array(val[0], val[1], val[2], val[3], val[4])
            localstr += string

    globalstr = ''
    headerbody = ''
    for k in contents:
        type = k[0]
        name = k[1]
        format = k[2]
        length = k[3]

        headerbody += f'extern {type} {name}[];\n'
        #print(f'NAME: {name}; FMT: {format}; DATA: {data[name]}')
        arr = generate_array(type, name, format, data[name], length)
        
        globalstr += arr
        

    headerbody = headerbody[:-1]
    headertxt = c_header_file(filenames[0], ['content_base.h'], headerbody)
    includetxt = f'#include "{filenames[0]}"\n'
    for k in saveconfig['includes']:
        includetxt += f'#include "{k}"\n'
    
    sourcetxt = f'{includetxt}{localstr}{globalstr}\n'
    
    updatefile(headerpath, headertxt)
    updatefile(sourcepath, sourcetxt)

def gen_res_font(asset, basedir):
    s = []
    name = asset['Name']
    tilesets = asset['Tilesets']
    for k in range(len(tilesets)):
        tileset = tilesets[k]
        path = tileset['Path']
        compr = tileset['Compression'].upper()
        s.append(f'TILESET tls_fnt_{name}_{k} "{path}" {compr} NONE')
    return s
def gen_res_map(asset, basedir):
    s = []
    name = asset['Name']
    path = asset['Path']
    mapjsonpath = os.path.splitext(os.path.normpath(os.path.join(basedir, path)))[0] + ".json"
    mapjson = readjson(mapjsonpath)
    mapwidth = mapjson['width']
    if mapwidth % 2 == 0:
        s.append(f"PALETTE pal_{name} \"{path}\"")
        s.append(f"TILESET tls_{name} \"{path}\" NONE ALL")
        s.append(f"MAP map_{name} \"{path}\" tls_{name} NONE")
    return s
def generator_resource(asset, basedirs):
    data_local = []
    data_global = []
    type = asset['Type']
    name = asset['Name']
    x = {
        'Image':    lambda a, b: [ f'IMAGE img_{name} "{a["Path"]}" {a["Compression"].upper()}' ],
        'Tileset':  lambda a, b: [ f'TILESET tls_{name} "{a["Path"]}" {a["Compression"].upper()} {a["Optimization"].upper()}'],
        'Sprite':   lambda a, b: [ f'SPRITE spr_{name} "{a["Path"]}" {a["Width"]} {a["Height"]} {a["Compression"].upper()} 0 {a["Collision"].upper()} {a["Optimization"].upper()} 500000' ],
        'Face':     lambda a, b: [ f'SPRITE spr_{name} "{a["Path"]}" {a["Width"]} {a["Height"]} {a["Compression"].upper()} 0 {a["Collision"].upper()} {a["Optimization"].upper()} 500000' ],
        'Palette':  lambda a, b: [ f'PALETTE pal_{name} "{a["Path"]}"' ],
        'Font':     gen_res_font,
        'Map':      gen_res_map
    }
    if type in x:
        data_local += x[type](asset, basedirs[0])
    print(data_local)
    return data_local, data_global

def generator_font(asset, basedirs):
    name = asset['Name']
    tilesets = asset['Tilesets']
    data = [ len(tilesets) ]
    for k in range(len(tilesets)):
        tileset = tilesets[k]
        data.append(f'&tls_fnt_{name}_{k}')
    
    for k in range(2-len(tilesets)):
        data.append('NULL')

    data_local = []
    data_global = [ ('content_fonts', [data]) ]
    
    return data_local, data_global

def generator_face(asset, basedir):
    data_local = []
    data_global = [ ('content_faces', [f'spr_{asset["Name"]}'])]
    return data_local, data_global
# main script function (yes i know this is not "pythonic" but it looks better to my eyes than sticking everything under the if statement)
def build_project_from_json_file(filepath):
    projectdir = os.path.dirname(filepath)
    project = readjson(filepath)
    
    resdir = os.path.normpath(os.path.join(projectdir, project['ExportResourceDirectory']))
    hdir = os.path.normpath(os.path.join(projectdir, project['ExportHeaderDirectory']))
    cdir = os.path.normpath(os.path.join(projectdir, project['ExportSourceDirectory']))
    
    _ASSET_DATA = [
        [ 
            [resdir],
            ['resources.res'],
            ['Image', 'Tileset', 'Sprite', 'Palette', 'Map', 'Font', 'Face', 'World'],
            saver_single_text,  generator_resource,
            { }
        ],
#        [ 
#            [hdir, cdir, resdir], # resdir needed by generator_map
#            ['content/maps.h', 'content/maps.c'],
#            ['Map'],
#            saver_c, generator_map,
#            { 
#                'contents': [
#                    ('const MapHeader_t', 'content_maps', '{{ {0[0]}, {0[1]}, {0[2]}, {0[3]}, {0[4]}, {0[5]}, {0[6]}, {0[7]}, {0[8]}, {0[9]} }}', 1)
#                ],
#                'includes': [ 'resources.h' ]
#            } 
#        ],
#        [ 
#            [hdir, cdir, resdir], # resdir needed by generator_map
#            ['content/maps.h', 'content/maps.c'],
#            ['Map'],
#            saver_c, generator_map,
#            { 
#                'contents': [
#                    ('const MapHeader_t', 'content_maps', '{{ {0[0]}, {0[1]}, {0[2]}, {0[3]}, {0[4]}, {0[5]}, {0[6]}, {0[7]}, {0[8]}, {0[9]} }}', 1)
#                ],
#                'includes': [ 'resources.h' ]
#            } 
#        ],
        [ 
            [hdir, cdir],
            ['content/fonts.h', 'content/fonts.c'],
            ['Font'],
            saver_c, generator_font,
            { 
                'contents': [
                    ('const FontHeader_t',  'content_fonts', '{{ {0[0]}, {0[1]}, {0[2]} }}', 1)
                ],
                'includes': [ 'resources.h' ]
            } 
        ],
        [
            [hdir, cdir, resdir],
            ['content/faces.h', 'content/faces.c'],
            ['Face'],
            saver_c, generator_face,
            {
                'contents': [
                    ('const SpriteDefinition*', 'content_faces', '&{0}', 1)
                ],
                'includes': [ 'resources.h' ]
            }
        ]
    ]

    filedata = {}
    for asset in project['Assets']:
        type = asset['Type']
        for i in range(len(_ASSET_DATA)):
            l = _ASSET_DATA[i]
            if type in l[2]:
                if not (i in filedata.keys()):
                    filedata[i] = [ [], [] ]

                localdata, globaldata = l[4](asset, l[0])
                filedata[i][0].append(localdata)
                filedata[i][1] += globaldata
    
    for i in range(len(_ASSET_DATA)):
        l = _ASSET_DATA[i]
        basedirs = l[0]
        filenames = l[1]
        restypes = l[2]
        saver = l[3]
        saveconfig = l[5]
        saver(basedirs, filenames, filedata[i], saveconfig)
if __name__ == "__main__":
    build_project_from_json_file(sys.argv[1])