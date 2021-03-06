project('PCSX2-MultiConfigTool',
        'cpp', 'c',
        meson_version : '>=0.52.0')

#Setup local variables
IF_Lic = './' + 'LICENSE'
OF_Lic = meson.current_build_dir() + '/license.h' #The builddir doesn't need including via 'include_directories', so the file is found by the compiler

src_tool = ['./src/appconfig.c',
            './src/inifile.c',
            './src/pcsx2_multiconfigtool.cpp']
inc_tool = include_directories('./src')

src_gui = ['./src/UI_wx/wxwin_functions.cpp',
           './src/UI_wx/wxwin_main.cpp',
           './src/UI_wx/wxwin_selectpaths.cpp']
inc_gui = include_directories('./src/UI_wx')

inc_res = include_directories('./resources')

cmdparam_xxd = ['-i',
                '-u',
                IF_Lic,
                OF_Lic]

cmdparam_sed = ['-i',
                '-e',
                '1i /**\\n * This File is autogenerated by the buildsystem\\n * DO NOT EDIT!\\n */',
                '-e',
                '/char/ c static const unsigned char ucaLicense_m[] ={',
                '-e',
                's/};/ ,0x00};/',
                '-e',
                '$d',
                OF_Lic]

if build_machine.system() == 'windows'
    #compile static wxWidgets on windows

    add_project_link_arguments('--static',
                               language : ['cpp']) #This is needed to also link other libraries static

    wxDep = dependency('wxwidgets',
                       version : '>=3.0.0',
                       method : 'config-tool',
                       static : true, # this is currently not working
                       required : true,
                       modules : ['--static', 'std', 'stc']) #the '--static' is a temporary hack to enable static linking

    wxResCmpArgs = []

    foreach str : wxDep.get_configtool_variable('rescomp').strip().split()
        if str.to_lower() == 'windres'
            continue  #skip windres executable name
        endif
        wxResCmpArgs += str
    endforeach

    mod_rescomp = import('windows')
    src_gui += mod_rescomp.compile_resources('./resources/pcsx2tool_theme_win32.rc',
                                             args : [wxResCmpArgs])

else
    wxDep = dependency('wxwidgets',
                       version : '>=3.0.0',
                       required : true,
                       modules : ['std', 'stc'])
endif

#generate license file for including
run_command('xxd',
            cmdparam_xxd,
            check : true)

#modify generated licensefile
run_command('sed',
            cmdparam_sed,
            check : true)

executable('PCSX2-MulticonfigTool',
           sources : [src_gui, src_tool],
           include_directories : [inc_gui, inc_tool, inc_res],
           dependencies : wxDep,
           gui_app : true)
