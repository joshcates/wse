#/*=========================================================================
#
#  Program:   Insight Segmentation & Registration Toolkit
#  Module:    $RCSfile: EditorModule.tcl,v $
#  Language:  C++
#  Date:      $Date: 2004-03-17 20:57:09 $
#  Version:   $Revision: 1.6 $
#
#  Copyright (c) 2002 Insight Consortium. All rights reserved.
#  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
#
#     This software is distributed WITHOUT ANY WARRANTY; without even 
#     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
#     PURPOSE.  See the above copyright notices for more information.
#
#=========================================================================*/

package require vtkinteraction
package require segmenterinteraction

###
#
set EditorGlobals(source_label)      "Segmentation"
set EditorGlobals(tree_source_label) "Tree"
set EditorGlobals(frame_title_text)  "Editor"
set EditorGlobals(source_tag) $SegmenterGlobals(segmentation_tag)
set EditorGlobals(tree_tag) $SegmenterGlobals(tree_tag)
set EditorGlobals(original_tag) ""
set EditorGlobals(source_tag_label)  "Using basic segmentation: "
set EditorGlobals(tree_tag_label)    "Using segmentation tree: "
set EditorGlobals(original_tag_label) "Using source data: "
set EditorGlobals(original_label)     "Source data"
set EditorGlobals(console_title) "Editor Console"
set EditorGlobals(help_title)  "Help"
set EditorGlobals(slice_label) "Slice number"
set EditorGlobals(merge_resolution_label) "Scale units"
set EditorGlobals(merge_label) "Scale"
set EditorGlobals(zoom_label) ""
set EditorGlobals(basemergeResolution) 0.00001
set EditorGlobals(mergeMagnifier)      200
set EditorGlobals(scaleLevel)          0
set EditorGlobals(previousScale)       0
set EditorGlobals(zoomLevel)           $Options(default_editor_magnification)
set EditorGlobals(zoomVal)             1
set EditorGlobals(overlayVal)          1
set EditorGlobals(paintRadiusValue)    0.0
set EditorGlobals(render_window_is_visible) "false"
set EditorGlobals(console_is_visible) "false"
set EditorGlobals(help_is_visible) "false"
set EditorGlobals(default_lower_image_threshold) 0.0
set EditorGlobals(default_upper_image_threshold) 255.0
set EditorGlobals(default_window)  256.0
set EditorGlobals(default_level)   127.5
set EditorGlobals(help_file) "EditorHelp.txt"
set EditorGlobals(help_text) "Sorry, could not find the file $EditorGlobals(help_file)"
set EditorGlobals(binary_volume_tag) "mask"


proc ConstructEditorFrame {f} {
    global Options EditorGlobals DataGlobals

    #
    # Create the detached editor console
    EditorCreateEditorConsole $f
    
    #
    # Create the editor module controls in the console
    # Create the GUI
    $f configure -background $Options(text_background_color)
    
    # Some title text
    frame $f.title -background $Options(pleasant_background_color)
    label $f.title.text -textvariable EditorGlobals(frame_title_text) \
        -background $Options(pleasant_background_color) \
        -font $Options(h1_font)
    pack $f.title.text -side left

    # Original data selector
    frame $f.original_frame
    frame $f.original_frame.loadframe
    label $f.original_frame.label1 -font $Options(h3_font) -text $EditorGlobals(original_label)
    listbox $f.original_frame.loadframe.listbox -selectmode single -width 20 \
        -height $Options(data_window_size) \
        -yscrollcommand "$f.original_frame.loadframe.scrollbar set"
    scrollbar $f.original_frame.loadframe.scrollbar -orient "vert" \
        -command "$f.original_frame.loadframe.listbox yview" 
    pack $f.original_frame.loadframe.scrollbar -side right -fill y
    pack $f.original_frame.loadframe.listbox

    RegisterDataSelector $f.original_frame.loadframe.listbox $DataGlobals(scalar_type)
    set EditorGlobals(original_selector) $f.original_frame.loadframe.listbox

    button $f.original_frame.select_source -text "select" -command \
        { set EditorGlobals(original_tag) [$EditorGlobals(original_selector) get [$EditorGlobals(original_selector) curselection] ] }

    grid configure $f.original_frame.label1 -padx 3 -column 0 -row 0 \
        -columnspan 1 -sticky nw
    grid configure $f.original_frame.loadframe -padx 3 -column 0 -row 1 \
        -columnspan 1 -sticky nw
    grid configure $f.original_frame.select_source   -padx 3 -column 0 -row 2 -sticky nw

    # Source selector
    frame $f.data_source
    frame $f.data_source.loadframe
    label $f.data_source.label1 -font $Options(h3_font) -text $EditorGlobals(source_label)
    listbox $f.data_source.loadframe.listbox -selectmode single -width 20 \
        -height $Options(data_window_size) \
        -yscrollcommand "$f.data_source.loadframe.scrollbar set"
    scrollbar $f.data_source.loadframe.scrollbar -orient "vert" \
        -command "$f.data_source.loadframe.listbox yview" 
    pack $f.data_source.loadframe.scrollbar -side right -fill y
    pack $f.data_source.loadframe.listbox

    RegisterDataSelector $f.data_source.loadframe.listbox $DataGlobals(labeled_image_type)
    set EditorGlobals(data_selector) $f.data_source.loadframe.listbox

    # Tree selector
    frame $f.data_source.loadframe_tree
    label $f.data_source.label2 -font $Options(h3_font) -text $EditorGlobals(tree_source_label)
    listbox $f.data_source.loadframe_tree.listbox -selectmode single -width 20 \
        -height $Options(data_window_size) \
        -yscrollcommand "$f.data_source.loadframe_tree.scrollbar set"
    scrollbar $f.data_source.loadframe_tree.scrollbar -orient "vert" \
        -command "$f.data_source.loadframe_tree.listbox yview" 
    pack $f.data_source.loadframe_tree.scrollbar -side right -fill y
    pack $f.data_source.loadframe_tree.listbox

    RegisterDataSelector $f.data_source.loadframe_tree.listbox $DataGlobals(tree_type)
    set EditorGlobals(tree_selector) $f.data_source.loadframe_tree.listbox

    button $f.data_source.select_source -text "select" -command \
        { set EditorGlobals(source_tag) [$EditorGlobals(data_selector) get [$EditorGlobals(data_selector) curselection] ] }

    button $f.data_source.select_tree -text "select" -command \
        { set EditorGlobals(tree_tag) [$EditorGlobals(tree_selector) get [$EditorGlobals(tree_selector) curselection] ] }
 
    grid configure $f.data_source.label1 -padx 3 -column 0 -row 0 \
        -columnspan 1 -sticky nw
    grid configure $f.data_source.label2 -padx 3 -column 1 -row 0 \
        -columnspan 1 -sticky nw
    grid configure $f.data_source.loadframe -padx 3 -column 0 -row 1 \
        -columnspan 1 -sticky nw

    grid configure $f.data_source.loadframe_tree -padx 3 -column 1 -row 1 \
        -columnspan 1 -sticky nw
    grid configure $f.data_source.select_source -padx 3 -column 0 -row 2 -sticky nw
    grid configure $f.data_source.select_tree   -padx 3 -column 1 -row 2 -sticky nw

    #
    # start frame
    frame $f.start_frame
    label $f.start_frame.original_data_label -text $EditorGlobals(original_tag_label)
    label $f.start_frame.selected_data_label -text $EditorGlobals(source_tag_label)
    label $f.start_frame.selected_tree_label -text $EditorGlobals(tree_tag_label)
    label $f.start_frame.original_data -textvariable EditorGlobals(original_tag)
    label $f.start_frame.selected_data -textvariable EditorGlobals(source_tag)
    label $f.start_frame.selected_tree -textvariable EditorGlobals(tree_tag)

    button $f.start_frame.start -text "Start" -command \
        {SpawnProcess "EditorStartEditor" "EditorRecoverFailedEditor"}
    set EditorGlobals(start_button) $f.editor.start

    grid configure $f.start_frame.original_data_label -padx 3 -column 0 -row 0 -sticky nw
    grid configure $f.start_frame.selected_data_label -padx 3 -column 0 -row 1 -sticky nw
    grid configure $f.start_frame.selected_tree_label -padx 3 -column 0 -row 2 -sticky nw

    grid configure $f.start_frame.original_data -padx 3 -column 1 -row 0 -sticky nw
    grid configure $f.start_frame.selected_data -padx 3 -column 1 -row 1 -sticky nw
    grid configure $f.start_frame.selected_tree -padx 3 -column 1 -row 2 -sticky nw

    grid configure $f.start_frame.start -padx 3 -column 0 -row 3 -sticky nw

 # Create a little box for instructions
    frame $f.instructions
      text $f.instructions.text \
        -font $Options(mono_font) \
        -width 75 \
        -height 10 \
        -background $Options(help_box_color) \
        -wrap "word" \
        -insertontime 0 \
        -setgrid true  \
        -yscrollcommand "$f.instructions.scrollbar set"

    scrollbar $f.instructions.scrollbar -orient "vert" -command "$f.instructions.text yview"

    if {[file exists $EditorGlobals(help_file)] == 1} {
        set fileId [open $EditorGlobals(help_file) r]
        
        if {$fileId != ""} {
            set i 0
            while {! [eof $fileId]} {
                set i [expr $i +1 ]
                set tmp [gets $fileId]
                append tmp "\n"
                [subst {$f.instructions.text}] insert $i.0 $tmp
            }
        }
    } else { 
        [subst {$f.instructions.text}] insert 0.0 $EditorGlobals(help_text)
    }
    
    pack $f.instructions.scrollbar -side right -fill y
    pack $f.instructions.text -fill both -expand t

    $f.instructions.text configure -state disabled


    # Configure layout
    grid configure $f.title -row 0 -column 0 -columnspan 2 -sticky nw -padx 10
    grid configure $f.original_frame -row 1 -column 0 -sticky nw -padx 10 -pady 10
    grid configure $f.data_source -row 1 -column 1 -sticky nw -padx 10 -pady 10
    grid configure $f.start_frame -row 2 -column 0 -columnspan 2 -sticky nw -padx 10 -pady 10
    grid configure $f.instructions -row 3 -column 0 -sticky nw -padx 3 -pady 3 \
        -columnspan 2

}

proc EditorCreateEditorConsole {f} {
    global EditorGlobals Options

    #
    # The Help window
    #
    toplevel $f.help -borderwidth 0
    set EditorGlobals(help_window) $f.help
    wm title $f.help $EditorGlobals(help_title)
    wm protocol $f.help WM_DELETE_WINDOW { EditorHideInfo }
    
    canvas $f.help.img -height [image height $Options(editor_help_image)] \
        -width [image width $Options(editor_help_image)]
    $f.help.img create image 0 0 -image $Options(editor_help_image) -anchor nw
    pack $f.help.img
    wm withdraw $f.help
    wm resizable $f.help 0 0

    #
    # The main editor console
    toplevel $f.console
    set EditorGlobals(console_window) $f.console
    wm title $f.console $EditorGlobals(console_title)
    wm resizable $f.console 0 0
    wm protocol $f.console WM_DELETE_WINDOW { EditorQuit }
    wm withdraw $f.console
    
    #
    # Free-floating render window
    toplevel $f.renderWindow
    set EditorGlobals(render_window) $f.renderWindow
    wm protocol $f.renderWindow WM_DELETE_WINDOW { EditorHideRenderWindow }
    wm withdraw $f.renderWindow
    wm title $f.renderWindow "Surface Rendering"

    frame $f.renderWindow.windowsFrame -relief ridge -bd 3 \
        -background \#999999 -cursor crosshair
    vtkTkRenderWidget $f.renderWindow.windowsFrame.r1 \
        -width 256 -height 256 
    button $f.renderWindow.hide -text "hide window" -command EditorHideRenderWindow

    checkbutton $f.renderWindow.antialias  -indicatoron t \
        -text "toggle antialiasing" -command  "EditorToggleAntialiasing"  \
        -variable EditorGlobals(antialiasing_on) -onvalue 1 -offvalue 0
    set EditorGlobals(antialiasing_on) 0

    checkbutton $f.renderWindow.antialias2  -indicatoron t \
        -text "toggle vtk antialiasing" -command  "EditorToggleVTKAntialiasing"  \
        -variable EditorGlobals(vtk_antialiasing_on) -onvalue 1 -offvalue 0
    set EditorGlobals(vtk_antialiasing_on) 0

    entry $f.renderWindow.addrenentry -width 5 -justify r
    set EditorGlobals(addrenentry) $f.renderWindow.addrenentry
    button $f.renderWindow.addrenentryButton -text "add renderer" -command EditorAddRenderer

    set EditorGlobals(render_widget) $f.renderWindow.windowsFrame.r1

    pack $f.renderWindow.windowsFrame     -padx 3 -pady 3 -fill both -expand t
    pack $f.renderWindow.windowsFrame.r1  -padx 3 -pady 3 -fill both -expand t
    pack $f.renderWindow.antialias        -padx 3 -pady 3
    pack $f.renderWindow.antialias2        -padx 3 -pady 3
    pack $f.renderWindow.addrenentry      -padx 3 -pady 3
    pack $f.renderWindow.addrenentryButton -pady 3 
    pack $f.renderWindow.hide             -padx 3 -pady 3

    # Set up the viewer windows.
    frame $f.console.windowsFrame -relief ridge -bd 3 -background \#999999 -cursor crosshair
    vtkTkImageViewerWidget $f.console.windowsFrame.r1 
    vtkTkImageViewerWidget $f.console.windowsFrame.r2 
    vtkTkImageViewerWidget $f.console.windowsFrame.r3 

    set EditorGlobals(segmentation_widget) $f.console.windowsFrame.r1
    set EditorGlobals(source_widget) $f.console.windowsFrame.r2
    set EditorGlobals(binary_widget) $f.console.windowsFrame.r3

    grid configure $f.console.windowsFrame.r1 -row 0 -column 0 -padx 3
    grid configure $f.console.windowsFrame.r2 -row 0 -column 1 -padx 3
    grid configure $f.console.windowsFrame.r3 -row 0 -column 2 -padx 3


#    pack $f.console.windowsFrame.r1 $f.console.windowsFrame.r2 $f.console.windowsFrame.r2 \
#        -side left -expand t -fill both


    #
    #
    EditorInitialize

    ######## Top row controls
    frame $f.console.controls1

    # Slice and merge level sliders
    frame $f.console.controls1.sliceFrame
    scale $f.console.controls1.sliceFrame.slice -orient horizontal -command EditorSetSlice \
        -variable EditorGlobals(sliceNumber) -label $EditorGlobals(slice_label)
    set EditorGlobals(sliceNumber) 0
    set EditorGlobals(slice_slider) $f.console.controls1.sliceFrame.slice

    scale $f.console.controls1.sliceFrame.mergemag -from 1 -to 200 \
        -variable EditorGlobals(mergeMagnifier) \
        -resolution 1  -length 75 -orient vertical \
        -command EditorSetMergeResolution \
        -label $EditorGlobals(merge_resolution_label) \
        -showvalue f

    set EditorGlobals(merge_mag_scale) $f.console.controls1.sliceFrame.mergemag

    scale $f.console.controls1.sliceFrame.merge -from 0.0 -to 1.0\
        -length 500 \
        -resolution 0.001 \
        -orient horizontal \
        -variable EditorGlobals(scaleLevel) \
        -label $EditorGlobals(merge_label) \
        -command EditorSetMerge

    set EditorGlobals(merge_scale) $f.console.controls1.sliceFrame.merge

    pack  $f.console.controls1.sliceFrame.slice    -side left -padx 3 -pady 3 -fill both -expand f
    pack  $f.console.controls1.sliceFrame.merge    -side left -padx 3 -pady 3 -fill both -expand f
    pack  $f.console.controls1.sliceFrame.mergemag -side left -padx 3 -pady 3 -fill both -expand f


    # Add, subtract, etc. buttons
    frame $f.console.controls1.buttonsFrame

    button $f.console.controls1.buttonsFrame.add      -text "add selected region" \
        -command EditorAddSelectedRegion
    button $f.console.controls1.buttonsFrame.subtract -text "subtract selected region" \
        -command EditorSubtractSelectedRegion
    button $f.console.controls1.buttonsFrame.mergeselected -text "merge selected" \
        -command EditorMergeSelected
    button $f.console.controls1.buttonsFrame.undolastmerge -text "undo last merge" \
        -command EditorUndoLastMerge

    pack $f.console.controls1.buttonsFrame.add \
        $f.console.controls1.buttonsFrame.subtract \
        $f.console.controls1.buttonsFrame.mergeselected \
        $f.console.controls1.buttonsFrame.undolastmerge \
        -side left -anchor nw -padx 1

    pack $f.console.controls1.buttonsFrame $f.console.controls1.sliceFrame \
        -side top

    # Display frame
    frame $f.console.display

    # image zoom frame
    frame $f.console.display.zoomFrame
    
    button $f.console.display.zoomFrame.go \
        -command { EditorScaleZoom $EditorGlobals(zoomLevel) } \
        -text "zoom"

    scale $f.console.display.zoomFrame.zoomScale  \
        -from 0.1 -to 5.0 -resolution 0.1 -orient horizontal \
        -variable EditorGlobals(zoomLevel) \
        -label $EditorGlobals(zoom_label)
    
    checkbutton $f.console.display.zoomFrame.zoom  -indicatoron t \
        -text "toggle zoom" -command EditorToggleZoom \
        -variable EditorGlobals(zoomVal) -onvalue 1 -offvalue 0

    pack $f.console.display.zoomFrame.zoom $f.console.display.zoomFrame.zoomScale \
        $f.console.display.zoomFrame.go \
        -side left -anchor sw -padx 3 -pady 3


    # other display frame buttons
    button $f.console.display.recolor -text "randomize colors" -command EditorDoRecolor    

    checkbutton $f.console.display.toggleOverlay -text "toggle overlay" \
        -indicatoron t -command EditorToggleOverlay \
        -variable EditorGlobals(overlayVal) -onvalue 1 -offvalue 0

    frame $f.console.display.colormap
    entry $f.console.display.colormap.colormapLow  -width 6 -justify r
    entry $f.console.display.colormap.colormapHigh -width 6 -justify r
    
    $f.console.display.colormap.colormapHigh insert 0 $EditorGlobals(default_upper_image_threshold)
    $f.console.display.colormap.colormapLow insert 0 $EditorGlobals(default_lower_image_threshold)
    
    set cmdStr [subst -nocommands  { EditorResetLookupTable [$f.console.display.colormap.colormapLow get] [$f.console.display.colormap.colormapHigh get] } ]
    
    button $f.console.display.colormap.colormapButton -text "scalar colormap" \
        -command $cmdStr -font $Options(small_font)
    
    pack $f.console.display.colormap.colormapLow $f.console.display.colormap.colormapHigh \
        $f.console.display.colormap.colormapButton -side left -padx 3


    frame $f.console.display.windowlevel
    entry $f.console.display.windowlevel.window  -width 6 -justify r
    entry $f.console.display.windowlevel.level -width 6 -justify r
    
    $f.console.display.windowlevel.window insert 0 $EditorGlobals(default_window)
    $f.console.display.windowlevel.level insert 0 $EditorGlobals(default_level)
    
    set cmdStr [subst -nocommands  { EditorResetWindowLevel [$f.console.display.windowlevel.window get] [$f.console.display.windowlevel.level get] } ]
    
    button $f.console.display.windowlevel.windowlevelButton -text "window / level" \
        -command $cmdStr -font $Options(small_font)
    
    pack $f.console.display.windowlevel.window $f.console.display.windowlevel.level \
        $f.console.display.windowlevel.windowlevelButton -side left -padx 3


    pack $f.console.display.zoomFrame $f.console.display.recolor \
        $f.console.display.toggleOverlay $f.console.display.colormap \
        $f.console.display.windowlevel \
        -side top -pady 3


    # Some uncategorized buttons
    frame  $f.console.miscFrame

    button $f.console.miscFrame.render  -text "3D Render" -command EditorToggleAntialiasing ;#EditorRenderIsoSurface
    scale $f.console.miscFrame.paintRadius  -from 0.0 -to 20.0 -resolution 1 \
        -orient horizontal  -variable EditorGlobals(paintRadiusValue) \
        -command EditorSetPaintRadius -label "paint radius"
    entry $f.console.miscFrame.paintValue -width 4 -justify r
    set EditorGlobals(paintColorEntry) $f.console.miscFrame.paintValue
    button $f.console.miscFrame.changePaintValue -text "change paint value" \
        -command EditorChangePaintValue
    button $f.console.miscFrame.displayInfo -text "show help" -command EditorDisplayInfo
    button $f.console.miscFrame.hideInfo -text "hide help"  -command EditorHideInfo

    pack $f.console.miscFrame.render $f.console.miscFrame.displayInfo \
        $f.console.miscFrame.hideInfo $f.console.miscFrame.paintRadius $f.console.miscFrame.paintValue $f.console.miscFrame.changePaintValue \
        -side top -pady 3

    
    $f.console.miscFrame.paintValue insert 0 1

    # File operation buttons (save, load, clear, quit, etc)
    frame $f.console.fileops

    button $f.console.fileops.quit  -text quit -command EditorQuit \
        -foreground \#550022  -highlightbackground \#550022
    button $f.console.fileops.clear   -text "clear all"       \
        -command {if { [tk_messageBox -message "Erase the volume?" -icon warning -default "cancel" -type "okcancel"] == "ok"} {  EditorClear } }
    button $f.console.fileops.saveBinary -text "save"          -command EditorWriteBinaryVolume
    button $f.console.fileops.loadBinary -text "load"          -command EditorReadBinaryVolume

    pack $f.console.fileops.saveBinary $f.console.fileops.loadBinary \
        $f.console.fileops.clear $f.console.fileops.quit \
        -side top -pady 3

    # Pack the frames in the main window.
    grid configure $f.console.windowsFrame -row 0 -column 0 -columnspan 3 
    grid configure $f.console.controls1    -row 1 -column 0 -columnspan 3
    grid configure $f.console.miscFrame    -row 2 -column 0
    grid configure $f.console.display      -row 2 -column 1
    grid configure $f.console.fileops      -row 2 -column 2


    #itk logo
#    canvas $f.console.controls1.zoomFrame.img
#    $f.console.controls1.zoomFrame.img create image 0 0 -image $Options(itk_logo) -anchor nw

    BindTkSegmentationViewer $f.console.windowsFrame.r1 $EditorGlobals(resamplerSeg) \
        $EditorGlobals(labeledImgReader) $EditorGlobals(manager)
    BindTkSourceImageViewer  $f.console.windowsFrame.r2 $EditorGlobals(resamplerSeg) \
        $EditorGlobals(labeledImgReader) $EditorGlobals(manager) \
       $EditorGlobals(viewerSeg) $EditorGlobals(binaryVolume) $EditorGlobals(viewerBin)

    #    BindTkImageViewer     $f.console.windowsFrame.r3
    ::vtk::bind_tk_imageviewer_widget $f.console.windowsFrame.r3

    bind $f.console.windowsFrame.r2 <KeyPress-e> { }  ;# overridden 
    bind $f.console.windowsFrame.r3 <KeyPress-e> { }  ;# overridden 

    ::vtk::bind_tk_render_widget $f.renderWindow.windowsFrame.r1
}

proc EditorSanityCheckFiles {} {
    global EditorGlobals

    if {$EditorGlobals(source_tag) == ""} { return "Segmentation not selected." }
    if {$EditorGlobals(original_tag) == ""} { return "Source data not selected." }
    if {$EditorGlobals(tree_tag) == ""} { return "Tree not selected." }

    return 0
}

proc EditorSegmentationReaderLoadData {} {
    global EditorGlobals

    set tagName $EditorGlobals(source_tag) 
    set cacheEntry [GetDataCacheEntry $tagName]
    
    if {$EditorGlobals(X) != [GetCacheEntryValue $cacheEntry "size_x"]} {
        return "Segmentation dimensions do not match source dimensions"
    }
    if {$EditorGlobals(Y) != [GetCacheEntryValue $cacheEntry "size_y"]} {
        return "Segmentation dimensions do not match source dimensions"
    }
    if {$EditorGlobals(Z) != [GetCacheEntryValue $cacheEntry "size_z"]} {
        return "Segmentation dimensions do not match source dimensions"
    }

    set endianString "SetDataByteOrderTo"
    append endianString [GetCacheEntryValue $cacheEntry "endianness"]
    append endianString "Endian"
    $EditorGlobals(labeledImgReader) $endianString

    if {[GetCacheEntryValue $cacheEntry "data_type"] == "UnsignedLong"} {
#        set datatypeString "SetDataScalarType 9" 
    } else {
        set datatypeString "SetDataScalarTypeTo"
        append datatypeString [GetCacheEntryValue $cacheEntry "data_type"]
        $EditorGlobals(labeledImgReader) $datatypeString
    }

    $EditorGlobals(labeledImgReader) SetNumberOfScalarComponents \
        [GetCacheEntryValue $cacheEntry "number_of_components"]

    $EditorGlobals(labeledImgReader) SetDataExtent \
        0 [GetCacheEntryValue $cacheEntry "size_x"] \
        0 [GetCacheEntryValue $cacheEntry "size_y"] \
        0 [GetCacheEntryValue $cacheEntry "size_z"]

    $EditorGlobals(labeledImgReader) SetFileNameSliceOffset \
        [GetCacheEntryValue $cacheEntry "slice_offset"]
    $EditorGlobals(labeledImgReader) SetHeaderSize \
        [GetCacheEntryValue $cacheEntry "header_size"]

    if { [regexp {^[yY]} [GetCacheEntryValue $cacheEntry "multiple_files"] ] } {
        $EditorGlobals(labeledImgReader) SetFileDimensionality 2
        $EditorGlobals(labeledImgReader) SetFilePrefix \
            [GetCacheEntryValue $cacheEntry "filename"] 
        $EditorGlobals(labeledImgReader) SetFilePattern \
            [GetCacheEntryValue $cacheEntry "file_pattern"]
    } else {
        $EditorGlobals(labeledImgReader) SetFileDimensionality 3
        $EditorGlobals(labeledImgReader) SetFileName \
            [GetCacheEntryValue $cacheEntry "filename"]
    }

    $EditorGlobals(labeledImgReader) Modified

    ConstructProgressWindow sourceReaderProgressWindow
    $EditorGlobals(labeledImgReader) AddObserver ProgressEvent \
        {ProgressProc $EditorGlobals(labeledImgReader) .sourceReaderProgressWindow}
    [$EditorGlobals(labeledImgReader) GetOutput] SetUpdateExtentToWholeExtent
    $EditorGlobals(labeledImgReader) Update

    # Find the maximum value in the segmented image
    $EditorGlobals(statistics_filter) SetInput [$EditorGlobals(labeledImgReader) GetOutput]
    $EditorGlobals(statistics_filter) Update
    set EditorGlobals(maxLabel) [$EditorGlobals(statistics_filter) GetMaximum]
    set EditorGlobals(maxLabel) [expr $EditorGlobals(maxLabel) + 1]

    $EditorGlobals(resamplerSeg) SetInput [ $EditorGlobals(labeledImgReader) GetOutput ]
    destroy .sourceReaderProgressWindow

    return 0
}

proc EditorSourceReaderLoadData {} { 
    global EditorGlobals

    set tagName $EditorGlobals(original_tag) 
    set cacheEntry [GetDataCacheEntry $tagName]

    set EditorGlobals(X) [GetCacheEntryValue $cacheEntry "size_x"]
    set EditorGlobals(Y) [GetCacheEntryValue $cacheEntry "size_y"]
    set EditorGlobals(Z) [GetCacheEntryValue $cacheEntry "size_z"]

    set endianString "SetDataByteOrderTo"
    append endianString [GetCacheEntryValue $cacheEntry "endianness"]
    append endianString "Endian"
    $EditorGlobals(colorImgReader) $endianString

    set datatypeString "SetDataScalarTypeTo"
    append datatypeString [GetCacheEntryValue $cacheEntry "data_type"]

    $EditorGlobals(colorImgReader) SetNumberOfScalarComponents \
        [GetCacheEntryValue $cacheEntry "number_of_components"]
    $EditorGlobals(colorImgReader) $datatypeString
    $EditorGlobals(colorImgReader) SetDataExtent \
        0 [GetCacheEntryValue $cacheEntry "size_x"] \
        0 [GetCacheEntryValue $cacheEntry "size_y"] \
        0 [GetCacheEntryValue $cacheEntry "size_z"]
    $EditorGlobals(colorImgReader) SetFileNameSliceOffset \
        [GetCacheEntryValue $cacheEntry "slice_offset"]

    $EditorGlobals(colorImgReader) SetHeaderSize \
        [GetCacheEntryValue $cacheEntry "header_size"]

    if { [regexp {^[yY]} [GetCacheEntryValue $cacheEntry "multiple_files"] ] } {
        $EditorGlobals(colorImgReader) SetFileDimensionality 2
        $EditorGlobals(colorImgReader) SetFilePrefix \
            [GetCacheEntryValue $cacheEntry "filename"] 
        $EditorGlobals(colorImgReader) SetFilePattern \
            [GetCacheEntryValue $cacheEntry "file_pattern"]
    } else {
        $EditorGlobals(colorImgReader) SetFileDimensionality 3
        $EditorGlobals(colorImgReader) SetFileName \
            [GetCacheEntryValue $cacheEntry "filename"]
    }

    $EditorGlobals(colorImgReader) Modified

    ConstructProgressWindow sourceReaderProgressWindow
    $EditorGlobals(colorImgReader) AddObserver ProgressEvent \
        {ProgressProc $EditorGlobals(colorImgReader) .sourceReaderProgressWindow}
    [$EditorGlobals(colorImgReader) GetOutput] SetUpdateExtentToWholeExtent
    $EditorGlobals(colorImgReader) Update

    $EditorGlobals(resamplerCol) SetInput [ $EditorGlobals(colorImgReader) GetOutput ]

    destroy .sourceReaderProgressWindow
    return 0
}

proc EditorResetLookupTable {low high} {
    global EditorGlobals

    $EditorGlobals(lut)  RemoveAllPoints
    $EditorGlobals(lut)  AddRGBPoint $low  0.0 0.0 0.0
    $EditorGlobals(lut)  AddRGBPoint $high 1.0 1.0 1.0

    $EditorGlobals(viewerCol) Render
}

proc EditorResetWindowLevel {window level} {
    global EditorGlobals

    $EditorGlobals(viewerCol) SetColorWindow $window
    $EditorGlobals(viewerCol) SetColorLevel $level
    $EditorGlobals(viewerCol) Render
}


proc EditorStartEditor {} {
    global EditorGlobals Options

    #
    # Check that the files are compatible
    set ret [ EditorSanityCheckFiles ]
    if { $ret != 0 } { 
        WSTErrorMacro $ret
        return 
    }
    
    #
    # Connect the pipeline
    $EditorGlobals(mapToRGBA) SetInput [$EditorGlobals(resamplerSeg) GetOutput]
    $EditorGlobals(mapToRGBA) SetLookupTable [$EditorGlobals(manager) GetLookupTable]
    $EditorGlobals(colorMapImg) SetLookupTable $EditorGlobals(lut)
    $EditorGlobals(colorMapImg) SetInput [$EditorGlobals(resamplerCol) GetOutput]
    $EditorGlobals(colorMapBin) SetInput [$EditorGlobals(resamplerBin) GetOutput]
    $EditorGlobals(colorMapBin) SetLookupTable $EditorGlobals(lutBin)
    $EditorGlobals(viewerCol) SetInput [$EditorGlobals(colorMapImg) GetOutput]
    $EditorGlobals(viewerSeg) SetInput [$EditorGlobals(mapToRGBA) GetOutput]
    $EditorGlobals(viewerBin) SetInput [$EditorGlobals(colorMapBin) GetOutput]
    $EditorGlobals(overlayMapper) SetInput [$EditorGlobals(colorMapBin) GetOutput]
    $EditorGlobals(map1) SetInput [$EditorGlobals(marcher1) GetOutput]
    $EditorGlobals(renWin) AddRenderer $EditorGlobals(ren1)
    
    #
    # Read the data
    #    
    catch { EditorSourceReaderLoadData } ret
    if { $ret != 0 }  { 
        WSTErrorMacro "Error loading source data."
        WSTErrorMacro $ret
        EditorRecoverFailedEditor
        return 
    }

    catch { EditorSegmentationReaderLoadData } ret
    if { $ret != 0 }  { 
        WSTErrorMacro "Error loading segmentation data."
        WSTErrorMacro $ret
        EditorRecoverFailedEditor
        return 
    } 

    #
    # Configure the lookup table manager for colorizing the segmentation
    #
    set treeTagName $EditorGlobals(tree_tag) 
    set treeCacheEntry [GetDataCacheEntry $treeTagName]
    set tree_file_name [GetCacheEntryValue $treeCacheEntry "filename"]

    $EditorGlobals(manager) Initialize
    $EditorGlobals(manager) LoadTreeFile $tree_file_name
    $EditorGlobals(manager) SetNumberOfLabels $EditorGlobals(maxLabel)
    $EditorGlobals(manager) GenerateColorTable

    $EditorGlobals(mapToRGBA) SetLookupTable [$EditorGlobals(manager) GetLookupTable]

    #
    # Configure the binary volume
    #
    $EditorGlobals(binaryVolume) SetExtent 0 $EditorGlobals(X) \
        0 $EditorGlobals(Y) 0 $EditorGlobals(Z)
    $EditorGlobals(binaryVolume) SetUpdateExtent 0 $EditorGlobals(X) \
        0 $EditorGlobals(Y) 0 $EditorGlobals(Z)
    $EditorGlobals(binaryVolume) AllocateScalars
    $EditorGlobals(binaryVolume) Clear

    $EditorGlobals(resamplerBin) SetInput $EditorGlobals(binaryVolume)
    $EditorGlobals(antialias_caster) SetInput $EditorGlobals(binaryVolume)
    
    #
    # Initialize bounding boxes
    #
    $EditorGlobals(boundingBoxes) SetLabeledImage [$EditorGlobals(labeledImgReader) GetOutput]
    $EditorGlobals(boundingBoxes) GenerateBoundingBoxes

    #
    # Configure the viewer widgets
    #
    $EditorGlobals(segmentation_widget) configure \
        -height [expr $EditorGlobals(Y) * $Options(default_editor_magnification) ] \
        -width [expr $EditorGlobals(X) * $Options(default_editor_magnification) ]
    $EditorGlobals(binary_widget) configure \
        -height [expr $EditorGlobals(Y) * $Options(default_editor_magnification) ] \
        -width [expr $EditorGlobals(X) * $Options(default_editor_magnification) ]
    $EditorGlobals(source_widget) configure \
        -height [expr $EditorGlobals(Y) * $Options(default_editor_magnification) ] \
        -width [expr $EditorGlobals(X) * $Options(default_editor_magnification) ]
    

    $EditorGlobals(slice_slider) configure -length [expr 2 * $EditorGlobals(Z)] \
        -from 0 -to $EditorGlobals(Z)


    #
    # Raise the editor console
    #
    set EditorGlobals(console_is_visible) "true"
    wm deiconify $EditorGlobals(console_window) 
}

proc EditorRecoverFailedEditor {} {
    global EditorGlobals

    set EditorGlobals(console_is_visible) "false"
    wm withdraw $EditorGlobals(console_window) 
    set EditorGlobals(render_window_is_visible) "false"
    wm withdraw $EditorGlobals(render_window)
}


proc EditorInitialize {} {
    global EditorGlobals Options

    #### Data sources
    # A statistics filter
    vtkITKStatisticsImageFilterULUL EditorStatisticsFilter
    set EditorGlobals(statistics_filter) EditorStatisticsFilter

    # Reader for the source data
    vtkImageReader colorImgReader
    colorImgReader SetHeaderSize 0
    set EditorGlobals(colorImgReader) colorImgReader

    #
    # Reader for segmented data
    #
#    vtkPatchedImageReader labeledImgReader
    vtkImageReader labeledImgReader
    labeledImgReader SetFileDimensionality 3
    labeledImgReader SetDataScalarType 9
    set EditorGlobals(labeledImgReader) labeledImgReader

    #
    # Initialize a binary volume
    #
    vtkBinaryVolume binaryVolume
    binaryVolume SetExtent 0 1 0 1 0 1
    binaryVolume SetUpdateExtent 0 1 0 1 0 1
    binaryVolume SetLabelValue 1
    binaryVolume Clear
    set EditorGlobals(binaryVolume) binaryVolume
    

    #### Resamplers

    #
    # Resampler for resizing the segmentation data
    # 
    vtkImageResample resamplerSeg
    resamplerSeg InterpolateOff
    resamplerSeg SetAxisMagnificationFactor 0 $Options(default_editor_magnification)
    resamplerSeg SetAxisMagnificationFactor 1 $Options(default_editor_magnification)
    set EditorGlobals(resamplerSeg) resamplerSeg   

    #
    # Resampler for resizing the original data
    #
    vtkImageResample resamplerCol
    resamplerCol InterpolateOff
    resamplerCol SetAxisMagnificationFactor 0 $Options(default_editor_magnification)
    resamplerCol SetAxisMagnificationFactor 1 $Options(default_editor_magnification)
    set EditorGlobals(resamplerCol) resamplerCol
    
    #
    # Resampler for resizing the binary data
    #    
    vtkImageResample resamplerBin
    resamplerBin InterpolateOff
    resamplerBin SetAxisMagnificationFactor 0 $Options(default_editor_magnification)
    resamplerBin SetAxisMagnificationFactor 1 $Options(default_editor_magnification)
    set EditorGlobals(resamplerBin) resamplerBin


    #### Lookup tables

    #
    # Lookup table manager for colorizing segment table.
    #
    vtkWSLookupTableManager manager
    manager SetHighlightColor 1.0 1.0 1.0
    manager SetRepaintHighlights 1
    set EditorGlobals(manager) manager

    #
    # Lookup table for the binary volume
    #
    vtkLookupTable lutBin
    lutBin SetNumberOfTableValues 256
    lutBin SetTableRange 0 255
    lutBin SetTableValue 0 0.0 0.0 0.0 0.0
    lutBin SetTableValue 1 1.0 0.0 0.0 0.3
    lutBin SetTableValue 2 0.0 1.0 0.0 0.3
    lutBin SetTableValue 3 0.0 0.0 1.0 0.3
    lutBin SetTableValue 4 1.0 0.0 1.0 0.3
    lutBin SetTableValue 5 1.0 1.0 0.5 0.3
    lutBin SetTableValue 6 1.0 0.5 1.0 0.3
    lutBin SetTableValue 205 1.0 0.0 1.0 0.6

    set EditorGlobals(lutBin) lutBin

    #
    # Color transfer function for original data
    #
    vtkColorTransferFunction lut
    lut AddRGBPoint  $EditorGlobals(default_lower_image_threshold)  0.0 0.0 0.0
    lut AddRGBPoint  $EditorGlobals(default_upper_image_threshold)  1.0 1.0 1.0
    set EditorGlobals(lut) lut


    #### Color mappers

    #
    # Color mapper for segmentation data
    #
    vtkImageMapToColors mapToRGBA
#    mapToRGBA SetInput [resamplerSeg GetOutput]
    mapToRGBA SetOutputFormatToRGBA
#    mapToRGBA SetLookupTable [manager GetLookupTable]
    set EditorGlobals(mapToRGBA) mapToRGBA

    #
    # Color maper for the original data
    #
    vtkImageMapToColors colorMapImg
#    colorMapImg SetInput [resamplerCol GetOutput]
#    colorMapImg SetLookupTable lut
    colorMapImg SetOutputFormatToRGBA
    set EditorGlobals(colorMapImg) colorMapImg

    #
    # Color mapper for the binary volume
    #
    vtkImageMapToColors colorMapBin
#    colorMapBin SetInput [resamplerBin GetOutput]
#    colorMapBin SetLookupTable lutBin
    set EditorGlobals(colorMapBin) colorMapBin


    #### Image viewers

    #
    # Viewer for source data
    #
    set EditorGlobals(viewerCol) [$EditorGlobals(source_widget) GetImageViewer]
    $EditorGlobals(viewerCol) SetZSlice 0
    $EditorGlobals(viewerCol) SetColorWindow 255.0
    $EditorGlobals(viewerCol) SetColorLevel 127.5
 #   $EditorGlobals(viewerCol) SetInput [colorMapImg GetOutput]

    ExitRegisterViewer $EditorGlobals(viewerCol)

    #
    # Viewer for segmented data.
    #
    set EditorGlobals(viewerSeg) [$EditorGlobals(segmentation_widget) GetImageViewer]
    $EditorGlobals(viewerSeg) SetColorWindow 255.0
    $EditorGlobals(viewerSeg) SetColorLevel 127.5
    $EditorGlobals(viewerSeg) SetZSlice 0
#    $EditorGlobals(viewerSeg) SetInput [mapToRGBA GetOutput]

    ExitRegisterViewer $EditorGlobals(viewerSeg)

    #
    # Viewer for binary data
    #
    set EditorGlobals(viewerBin) [$EditorGlobals(binary_widget) GetImageViewer]
    $EditorGlobals(viewerBin) SetColorWindow 255.0
    $EditorGlobals(viewerBin) SetColorLevel 127.0
    $EditorGlobals(viewerBin) SetZSlice 0
 #   $EditorGlobals(viewerBin) SetInput [colorMapBin GetOutput]

    ExitRegisterViewer $EditorGlobals(viewerBin)

    #### Set up the binary image overlay
    
    #
    # Image mapper for the overlay
    #
    vtkImageMapper overlayMapper
  #  overlayMapper SetInput [colorMapBin GetOutput]
    overlayMapper SetColorWindow 255
    overlayMapper SetColorLevel 129
    overlayMapper SetZSlice 0
    set EditorGlobals(overlayMapper) overlayMapper
    
    #
    # Actor for the overlay
    #
    vtkActor2D overlayActor
    overlayActor SetMapper overlayMapper
    [$EditorGlobals(viewerCol) GetRenderer] AddActor2D overlayActor
    set EditorGlobals(overlayActor) overlayActor


    #### Initialize bounding box and binary volume logic

    #
    # Bounding box manager for the labeled data.
    #
    vtkWSBoundingBoxManager boundingBoxes
    boundingBoxes SetLabeledImage [labeledImgReader GetOutput]
    set EditorGlobals(boundingBoxes) boundingBoxes
    
    #
    # Logic object for the binary volume
    #
    vtkBinaryVolumeLogic logic
    set EditorGlobals(logic) logic


    #
    # Create a render window
    #
    set EditorGlobals(renWin) [$EditorGlobals(render_widget) GetRenderWindow]
#    $EditorGlobals(renWin) AddRenderer ren1
    $EditorGlobals(renWin) SetSize 256 256
    ExitRegisterViewer $EditorGlobals(renWin)

   #
    # Create a 3D renderer
    #
    vtkRenderer ren1
    ren1 SetViewport 0 0 1 1
    set EditorGlobals(ren1) ren1



    #### Set up the surface renderer
    AddSurfaceRenderer 1 0.5 0.5 0.0;

    #
    # Alternative vtk antialiasing filter
    #
    vtkWindowedSincPolyDataFilter EditorWindowedSincFilter
    EditorWindowedSincFilter SetNumberOfIterations 20
    set EditorGlobals(vtk_antialiaser) EditorWindowedSincFilter 
 

    #
    # Set up an antialiasing filter for the surface
    #
    vtkImageCast EditorAntialiasCaster
    EditorAntialiasCaster SetOutputScalarTypeToFloat
    set EditorGlobals(antialias_caster) EditorAntialiasCaster

    vtkITKAntiAliasBinaryImageFilter EditorAntialiaser
    EditorAntialiaser SetMaximumRMSError 0.02
    EditorAntialiaser SetNumberOfIterations 20
    set EditorGlobals(antialiaser) EditorAntialiaser
}

proc ActivateEditorModule {} {
    global EditorGlobals

    if { $EditorGlobals(console_is_visible) == "true" } {
        wm deiconify $EditorGlobals(console_window)
    }
    if { $EditorGlobals(render_window_is_visible) == "true" } {
        wm deiconify $EditorGlobals(render_window) 
    }
    if { $EditorGlobals(help_is_visible) == "true" } {
        wm deiconify $EditorGlobals(help_window) 
    }
}

proc DeactivateEditorModule {} {
    global EditorGlobals

    wm withdraw $EditorGlobals(console_window)
    wm withdraw $EditorGlobals(render_window)
    wm withdraw $EditorGlobals(help_window)
}

proc EditorClear { } {
    global EditorGlobals

    $EditorGlobals(binaryVolume) Clear
    $EditorGlobals(viewerBin)    Render
    $EditorGlobals(viewerCol)    Render
}

proc EditorDisplayInfo { } {
    global EditorGlobals

    set EditorGlobals(help_is_visible) "true"
    wm deiconify $EditorGlobals(help_window)
}
proc EditorHideInfo { } {
    global EditorGlobals

    set EditorGlobals(help_is_visible) "false"
    wm withdraw $EditorGlobals(help_window)
}

proc EditorUndoLastMerge { } {
    global EditorGlobals

    $EditorGlobals(manager) ClearHighlightedValuesToSameColor
    $EditorGlobals(manager) ClearComputedEquivalencyList

    set t [ $EditorGlobals(manager) UndoLastMerge ]

    set EditorGlobals(previousScale) $t
    set EditorGlobals(scaleLevel) $t

    $EditorGlobals(viewerSeg) Render
}

proc EditorSetPaintRadius { r } {
    global EditorGlobals

    $EditorGlobals(binaryVolume) SetPaintRadius $r
}

proc Editorlocal_min { a b } {
    if {$a < $b} { 
        return $a
    } else {
        return $b 
    }
}

proc Editorlocal_max { a b } {
    if {$a > $b} { 
        return $a
    } else {
        return $b 
    }
}
proc EditorSetMergeResolution { mag } {
    global EditorGlobals

    set length [ $EditorGlobals(merge_scale) cget -length ]

    set res [expr $EditorGlobals(basemergeResolution) * $mag]

    set min [ Editorlocal_max 0.0 [expr $EditorGlobals(scaleLevel) - ( ($length / 2) * $res )] ]

    set max [ Editorlocal_min 1.0 [expr $min + ($length * $res) ] ]
    
    $EditorGlobals(merge_scale) configure -resolution $res -from $min -to $max
}

proc EditorMergeSelected { } {
    global EditorGlobals

    $EditorGlobals(manager) ClearHighlightedValuesToSameColor ;#deselect regions
    set scaleLevel  [ $EditorGlobals(manager) MergeSelected ]
    $EditorGlobals(manager) RecompileEquivalencies
    $EditorGlobals(manager) HighlightComputedEquivalencyList
    $EditorGlobals(viewerSeg) Render
}

proc EditorHideRenderWindow { } {
    global EditorGlobals

    wm withdraw $EditorGlobals(render_window)
}

proc EditorRenderIsoSurface { } {
#    global EditorGlobals

#    wm deiconify $EditorGlobals(render_window)

    #render the antialiased volume
#    EditorToggleAntialiasing

    #render the rest 

    
    
}

proc EditorAddSelectedRegion { } {
    global EditorGlobals
    
    $EditorGlobals(logic) SetBinaryVolume $EditorGlobals(binaryVolume)
    $EditorGlobals(logic) SetSourceVolume [$EditorGlobals(labeledImgReader) GetOutput]
    $EditorGlobals(logic) AddEquivalencies $EditorGlobals(manager) $EditorGlobals(boundingBoxes)

    $EditorGlobals(viewerCol) Render
    $EditorGlobals(viewerBin) Render
}

proc EditorSubtractSelectedRegion { } {
    global EditorGlobals

    $EditorGlobals(logic) SetBinaryVolume $EditorGlobals(binaryVolume)
    $EditorGlobals(logic) SetSourceVolume [$EditorGlobals(labeledImgReader) GetOutput]
    $EditorGlobals(logic) SubtractEquivalencies $EditorGlobals(manager) $EditorGlobals(boundingBoxes)

    $EditorGlobals(viewerCol) Render
    $EditorGlobals(viewerBin) Render
}

proc EditorScaleZoom { mag } {
    global EditorGlobals Options
    if { $EditorGlobals(zoomVal) == 1 } {
        ;# Resample the data
        $EditorGlobals(resamplerSeg) SetAxisMagnificationFactor 0 $mag
        $EditorGlobals(resamplerSeg) SetAxisMagnificationFactor 1 $mag
        $EditorGlobals(resamplerCol) SetAxisMagnificationFactor 0 $mag
        $EditorGlobals(resamplerCol) SetAxisMagnificationFactor 1 $mag
        $EditorGlobals(resamplerBin) SetAxisMagnificationFactor 0 $mag
        $EditorGlobals(resamplerBin) SetAxisMagnificationFactor 1 $mag

        ;# Resize the windows if needed
        if { $mag * $EditorGlobals(Y) > $Options(default_editor_magnification) * $EditorGlobals(Y) } {
        $EditorGlobals(segmentation_widget) configure -width [expr $mag * $EditorGlobals(X)] \
            -height [expr $mag * $EditorGlobals(Y)]
        $EditorGlobals(source_widget) configure -width [expr $mag * $EditorGlobals(X)] \
            -height [expr $mag * $EditorGlobals(Y)]
        $EditorGlobals(binary_widget) configure -width [expr $mag * $EditorGlobals(X)] \
            -height [expr $mag * $EditorGlobals(Y)]
        } elseif { $mag != 1.0} { 
            $EditorGlobals(segmentation_widget) configure -width [expr $Options(default_editor_magnification) * $EditorGlobals(X)] \
                -height [expr $Options(default_editor_magnification) * $EditorGlobals(Y)]
            $EditorGlobals(source_widget) configure -width [expr $Options(default_editor_magnification) * $EditorGlobals(X)] \
                -height [expr $Options(default_editor_magnification) * $EditorGlobals(Y)]
            $EditorGlobals(binary_widget) configure -width [expr $Options(default_editor_magnification) * $EditorGlobals(X)] \
                -height [expr $Options(default_editor_magnification) * $EditorGlobals(Y)]
        }
       
        ;# Re-render
        $EditorGlobals(viewerSeg) Render
        $EditorGlobals(viewerCol) Render
        $EditorGlobals(viewerBin) Render
    }
}

proc EditorToggleZoom {} {
    global EditorGlobals
    if { $EditorGlobals(zoomVal) == 1} {
        EditorScaleZoom $EditorGlobals(zoomLevel)
    } else  {        
        set EditorGlobals(zoomVal) 1
        EditorScaleZoom 1.0
        set EditorGlobals(zoomVal) 0
    }
}

proc EditorSetSlice {slice} {
   global EditorGlobals

   $EditorGlobals(viewerSeg) SetZSlice $slice
   $EditorGlobals(viewerCol) SetZSlice $slice   
   $EditorGlobals(viewerBin) SetZSlice $slice
   $EditorGlobals(overlayMapper) SetZSlice $slice

   $EditorGlobals(viewerSeg) Render
   $EditorGlobals(viewerCol) Render
   $EditorGlobals(viewerBin) Render
}

proc EditorSetMerge {scale} {
    global EditorGlobals

    if { $scale == $EditorGlobals(previousScale) } { return }

    $EditorGlobals(manager) ClearHighlightedValuesToSameColor
    $EditorGlobals(manager) Merge $scale
    set EditorGlobals(previousScale) $scale;

    $EditorGlobals(viewerSeg) Render
}

proc EditorDoRecolor {} {
    global EditorGlobals
    $EditorGlobals(manager) RandomizeColors
    $EditorGlobals(viewerSeg) Render
}

proc EditorToggleOverlay {} {
 global EditorGlobals

    if { $EditorGlobals(overlayVal) == 1} {
        $EditorGlobals(overlayActor) VisibilityOn
    } else  {        
        $EditorGlobals(overlayActor) VisibilityOff
    }
    $EditorGlobals(viewerCol) Render
}

proc EditorQuit {} {
    global EditorGlobals Options

    if { [tk_messageBox -default cancel -type okcancel -message "Really quit?" -icon question] \
             == "cancel" } { return }

    if { $Options(force_write_on_editor_exit) == "true" } {
        EditorWriteBinaryVolume
    }

    wm withdraw $EditorGlobals(console_window)
    wm withdraw $EditorGlobals(render_window)
    wm withdraw $EditorGlobals(help_window)
    
    $EditorGlobals(binaryVolume) Clear
    
    update
}

proc EditorWriteBinaryVolume {} {
    global EditorGlobals DataGlobals

    set filename [tk_getSaveFile -initialdir $DataGlobals(cache_directory) \
                      -defaultextension ".binaryvolume" \
                      -filetypes [list "{Binary volumes} {.binaryvolume}"] ];

    set retVal [ $EditorGlobals(binaryVolume) WriteToDisk $filename ]

    if { $retVal == 0} { tk_messageBox -message "Save OK" -icon info }
    if { $retVal != 0} { tk_messageBox -message "Save failed." -icon "error" }

    #Register this saved data
    set size_x $EditorGlobals(X)
    set size_y $EditorGlobals(Y)
    set size_z $EditorGlobals(Z)
    set filename "$filename.data"

    set entryStr "$filename UnsignedChar $size_x $size_y $size_z $DataGlobals(default_file_endianness) No 1 $DataGlobals(default_file_pattern) 0 0"
    AddDataCacheEntry [string range $filename [expr [string last "/" $filename] + 1] [expr [string first "." $filename] - 1]] $entryStr
}

proc EditorReadBinaryVolume {} {
    global EditorGlobals DataGlobals

    set filename [tk_getOpenFile -initialdir $DataGlobals(cache_directory) \
                      -defaultextension ".binaryvolume" \
                      -filetypes [list "{Binary volumes} {.binaryvolume}"] ];
    
    set retVal [ $EditorGlobals(binaryVolume) ReadFromDisk $filename ]

    if { $retVal == 0} { tk_messageBox -message "Load OK" -icon info }
    if { $retVal == -2} { tk_messageBox -message "Volume dimensions do not match." \
                              -icon "error" }
    if { $retVal != 0} { tk_messageBox -message "Load failed." -icon "error" }

    $EditorGlobals(viewerBin) Render
    $EditorGlobals(viewerCol) Render
}


proc EditorToggleAntialiasing {} {
    global EditorGlobals

    ConstructProgressWindow antialiasProgressWindow

    wm withdraw $EditorGlobals(render_window)

    if {$EditorGlobals(antialiasing_on) == 1} {
        if { $EditorGlobals(vtk_antialiasing_on) == 1 } {
            $EditorGlobals(thresher1) SetInput $EditorGlobals(binaryVolume)
            $EditorGlobals(marcher1) SetInput [$EditorGlobals(thresher1) GetOutput]
            $EditorGlobals(vtk_antialiaser) SetInput [$EditorGlobals(marcher1) GetOutput]
            
            $EditorGlobals(vtk_antialiaser) AddObserver ProgressEvent \
                {ProgressProc $EditorGlobals(vtk_antialiaser) .antialiasProgressWindow}

            $EditorGlobals(map1) SetInput [$EditorGlobals(vtk_antialiaser) GetOutput]
            [$EditorGlobals(blob1) GetProperty] SetInterpolationToFlat

        } else {
            $EditorGlobals(map1) SetInput [$EditorGlobals(marcher1) GetOutput]
            $EditorGlobals(thresher1) SetInput $EditorGlobals(binaryVolume)
            $EditorGlobals(antialias_caster) SetInput [$EditorGlobals(thresher1) GetOutput]
            $EditorGlobals(antialias_caster) AddObserver ProgressEvent \
                {ProgressProc $EditorGlobals(antialias_caster) .antialiasProgressWindow}
            $EditorGlobals(antialias_caster) Update
            $EditorGlobals(antialiaser) SetInput [$EditorGlobals(antialias_caster) GetOutput]
            $EditorGlobals(antialiaser) AddObserver ProgressEvent \
                {ProgressProc $EditorGlobals(antialiaser) .antialiasProgressWindow}
            $EditorGlobals(antialiaser) Update
        
            $EditorGlobals(marcher1) SetInput [$EditorGlobals(antialiaser) GetOutput]
            $EditorGlobals(marcher1) AddObserver ProgressEvent \
                {ProgressProc $EditorGlobals(marcher1) .antialiasProgressWindow}
            [$EditorGlobals(blob1) GetProperty] SetInterpolationToGouraud

        }
    } else {
        $EditorGlobals(thresher1) SetInput $EditorGlobals(binaryVolume)
        
        $EditorGlobals(marcher1) SetInput [$EditorGlobals(thresher1) GetOutput]
        $EditorGlobals(map1) SetInput [$EditorGlobals(marcher1) GetOutput]
        $EditorGlobals(marcher1) AddObserver ProgressEvent \
            {ProgressProc $EditorGlobals(marcher1) .antialiasProgressWindow}
    }

    $EditorGlobals(renWin) Render

    $EditorGlobals(antialias_caster) AddObserver ProgressEvent {}
    $EditorGlobals(antialiaser) AddObserver ProgressEvent {}
    $EditorGlobals(marcher1) AddObserver ProgressEvent {}
    $EditorGlobals(vtk_antialiaser) AddObserver ProgressEvent {}

    destroy .antialiasProgressWindow

    wm deiconify $EditorGlobals(render_window)
}

proc AddSurfaceRenderer {isovalue r g b} {
    global EditorGlobals
    #### Set up the surface renderer
    
    set marchername "marcher"
    set mapname "map"
    set threshname "thresher"
    set blobname "blob"
    append blobname $isovalue
    append marchername $isovalue
    append mapname $isovalue
    append threshname $isovalue
    

    #
    # Threshold filter to remove other isovalues
    #
    vtkImageThreshold $threshname
    $threshname ThresholdBetween [expr $isovalue - 0.9] [expr $isovalue + 0.9]
#    $threshname SetReplaceOut 1
    $threshname SetOutValue 0.0
 #   $threshname SetReplaceIn 1
 #   $threshname SetInValue $isovalue
    set EditorGlobals($threshname) $threshname
    
    $threshname SetInput binaryVolume

    #
    # Marching cubes algorithm
    #
    vtkMarchingCubes $marchername
    $marchername SetNumberOfContours 1
    $marchername SetValue 0 $isovalue
   # $marchername SetInput binaryVolume
    set EditorGlobals($marchername) $marchername
    
    $marchername SetInput [$threshname GetOutput]

    #
    # Polygon mapper for surface rendering
    #
    vtkOpenGLPolyDataMapper $mapname
    $mapname ScalarVisibilityOff
    $mapname ImmediateModeRenderingOn
#    $mapname SetInput [$marchername GetOutput]
    set EditorGlobals($mapname) $mapname

 
    #
    # Create an actor for the surface
    #
    vtkActor $blobname
    $blobname SetMapper $mapname
    eval [$blobname GetProperty] SetColor $r $g $b
    $EditorGlobals(ren1) AddActor $blobname
    set EditorGlobals($blobname) $blobname

}

proc EditorAddRenderer {} {
    global EditorGlobals

    set isovalue [$EditorGlobals(addrenentry) get]
    set r  0.5; #[expr rand(1.0)]
    set g  0.2; #[expr rand(1.0)]
    set b  0.3; #[expr rand(1.0)]
    AddSurfaceRenderer $isovalue $r $g $b
    
    set marchername "marcher"
    set mapname "map"
    set threshname "thresher"
    append marchername $isovalue
    append mapname $isovalue
    append threshname $isovalue
    
    $EditorGlobals($threshname) SetInput $EditorGlobals(binaryVolume)
    $EditorGlobals($marchername) SetInput [$EditorGlobals($threshname) GetOutput]
    $EditorGlobals($mapname) SetInput [$EditorGlobals($marchername) GetOutput]
    
    EditorToggleAntialiasing
}

proc EditorToggleVTKAntialiasing {} {
    global EditorGlobals
    EditorToggleAntialiasing
}

proc EditorChangePaintValue {} {
    global EditorGlobals
    $EditorGlobals(binaryVolume) SetLabelValue [$EditorGlobals(paintColorEntry) get]    
}
