# Parameter 1 - Absolute path to workspace directory
if [ $# -eq 0 ]; then
    echo "Need to pass workspace directory to the script"
    exit 1
fi

# Environment Variable - QTVERSION - Version of Qt to build
if [[ -z "${QTVERSION}" ]]; then
    echo "QTVERSION is undefined. Example: export QTVERSION=5.15.2"
    exit 1
else
    echo "QTVERSION=${QTVERSION}"
fi

# Location of the workspace directory (root of the folder structure)
export WORKSPACE_DIR=$1

# Location of the source code directory (top of git tree - qt5.git)
export SOURCE_DIR=$WORKSPACE_DIR/src

# REM Location where the final build will be located, as defined by the -prefix option
export INSTALL_DIR=$WORKSPACE_DIR/install/qt_$QTVERSION

# Get the number of processors available to build Qt
export NUMBER_OF_PROCESSORS=`sysctl -n hw.ncpu`
echo "make -j$NUMBER_OF_PROCESSORS"

# Define the modules to skip (because they are under commercial license)
export MODULES_TO_SKIP="-skip qtnetworkauth -skip qtpurchasing -skip qtquickcontrols -skip qtquick3d -skip qtlottie -skip qtcharts -skip qtdatavis3d -skip qtvirtualkeyboard -skip qtscript -skip qtwayland -skip qtwebglplugin"

# Configure the build
# Configure options: https://wiki.qt.io/Qt_5.15_Tools_and_Versions
$SOURCE_DIR/configure -opensource -confirm-license -verbose -prefix $INSTALL_DIR -debug-and-release -force-debug-info -nomake tests -nomake examples -plugin-sql-sqlite -silent -no-strip -no-framework -opengl desktop -no-warnings-are-errors $MODULES_TO_SKIP
if [ $? -eq 0 ]; then    
    # Build 
    make -j$NUMBER_OF_PROCESSORS
    if [ $? -eq 0 ]; then        
        make install
        if [ $? -eq 0 ]; then
            # Generate and compress debug symbols in install directory
            cd $INSTALL_DIR
            
            for x in $(ls ./**/*.dylib); do
                if ! [ -L $x ]; then
                    echo Generating debug symbols for $x
                    dsymutil $x;
                    tar -czf $x.dSYM.tgz --directory $(dirname $x) $(basename $x).dSYM ;
                    rm -rf $x.dSYM;
                fi
            done

            for x in $(ls ./**/**/*.dylib); do
                if ! [ -L $x ]; then
                    echo Generating debug symbols for $x
                    dsymutil $x;
                    tar -czf $x.dSYM.tgz --directory $(dirname $x) $(basename $x).dSYM ;
                    rm -rf $x.dSYM;
                fi
            done

            # Remove the webkit webengine debug symbols because they are incredibly heavy,
            # more than half of the artifact.
            rm -vf lib/libQt5Web*.dSYM.tgz

            # Adjust RUNPATHs
            find . -name libQt?Core.$QTVERSION.dylib | xargs install_name_tool -rpath @executable_path/../Frameworks @loader_path/../MacOS
            if [ $? -ne 0 ]; then
                echo "**** Failed to set qtbase/core rpath ****"
                exit 1
            fi

            # Compress folders for Maya devkit
            tar -czf qt_$QTVERSION-include.tar.gz --directory=include/ . && \
            tar -czf qt_$QTVERSION-cmake.tar.gz --directory=lib/cmake/ . && \
            tar -czf qt_$QTVERSION-mkspecs.tar.gz --directory=mkspecs/ . && \
            echo "==== Success ====" || echo "**** Failed to create tar files ****"
        else
            echo "**** Failed to create install ****"
            exit 1
        fi
    else
        echo "**** Failed to build ****"
        exit 1
    fi
else
    echo "**** Failed to configure build ****"
    exit 1
fi