# PROJECT:        Deltarune MD
# FILE:           res/resources.res
# AUTHOR:         RatcheT2497
# CREATION:       ???
# MODIFIED:       25/09/21
# DESCRIPTION:    Resource file for the project, to be preprocessed with the ../tools/resgen.py script and then compiled with SGDK's rescomp.
#                 Not sure how to handle asset distribution, so the GitHub won't have anything other than this file just yet.
# CHANGELOG:      (23/09/21) Added this file header. -R#
#                 (24/09/21) Added collision data to torihouse_krisroom level.
#                            Added preprocessing for easy map creation
#                 (25/09/21) Added inline collision data to torihouse_krisroom & torihouse_hallway.
SPRITE spr_placeholder ".\sprites\placeholder.png" 4 6 NONE 0 NONE BALANCED 500000
PALETTE pal_placeholder_spr ".\sprites\placeholder.png"
SPRITE spr_kris ".\characters\kris.png" 4 5 NONE 0 NONE BALANCED 500000
PALETTE pal_main_party ".\characters\main_party_palette.png"
# level resources
TILESET tls_placeholder_map ".\maps\placeholder.png"
PALETTE pal_placeholder_map ".\maps\placeholder.png"
MAP map_placeholder ".\maps\placeholder.png" tls_placeholder_map
BIN cut_test ".\maps\cutscene.bin" NONE
TILESET tls_torihouse_krisroom ".\maps\torihouse_krisroom.png" NONE ALL
PALETTE pal_torihouse_krisroom ".\maps\torihouse_krisroom.png"
MAP map_torihouse_krisroom ".\maps\torihouse_krisroom.png" tls_torihouse_krisroom
BIN col_torihouse_krisroom maps\torihouse_krisroom.col 2 2 0 NONE
TILESET tls_torihouse_hallway ".\maps\torihouse_hallway.png" NONE ALL
PALETTE pal_torihouse_hallway ".\maps\torihouse_hallway.png"
MAP map_torihouse_hallway ".\maps\torihouse_hallway.png" tls_torihouse_hallway
BIN col_torihouse_hallway maps\torihouse_hallway.col 2 2 0 NONE
TILESET tls_torihouse_lobby ".\maps\torihouse_lobby.png" NONE ALL
PALETTE pal_torihouse_lobby ".\maps\torihouse_lobby.png"
MAP map_torihouse_lobby ".\maps\torihouse_lobby.png" tls_torihouse_lobby
TILESET tls_torihouse_bathroom ".\maps\torihouse_bathroom.png" NONE ALL
PALETTE pal_torihouse_bathroom ".\maps\torihouse_bathroom.png"
MAP map_torihouse_bathroom ".\maps\torihouse_bathroom.png" tls_torihouse_bathroom
# editor resources
TILESET tls_editor_font ".\fonts\editor_font.png" NONE NONE
SPRITE spr_crosshair ".\sprites\crosshair.png" 2 2 NONE 0 NONE BALANCED 500000
SPRITE spr_selector_small ".\sprites\selector_small.png" 1 1 NONE 0 NONE BALANCED 500000
