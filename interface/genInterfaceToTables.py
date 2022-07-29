import em, os

all_files = os.listdir("./definitions")

for file_name in all_files:

    f = open("./definitions/" + file_name, 'r')

    column_info = []
    db_table_name = f.readline().rstrip("\n")
    for line in f.readlines():
        if line == "\n":
            continue
        col = {}
        items = line.split(" ")
        col["data_type"] = items[0]
        col["name"] = items[1].rstrip("\n")
        col["default"] = "NULL"
        col["conversion"] = items[2].rstrip("\n")
        col["qt_type"] = items[3].rstrip("\n")
        column_info.append(col)

    if file_name.endswith(".txt"):
        print(file_name)
        table_name = file_name.split(".")[0]
        class_name = table_name.capitalize() + "Interface"
        item_class_name = table_name.capitalize() + "Item1"
        class_header_name = table_name + "interface.hpp"
        database_manager_name = "DbManager"
        glob = {"db_table" : db_table_name, "database_manager_name" : database_manager_name,"class_name": class_name , "column_info" : column_info, "table_name" : table_name, "class_header_name" : class_header_name, "struct_name" : table_name.capitalize() + "Info", "item_class_name" : item_class_name}
        glob_header = {"db_table" : db_table_name, "database_manager_name" : database_manager_name,"class_name": class_name , "column_info" : column_info, "table_name" : table_name, "class_header_name" : class_header_name, "struct_name" : table_name.capitalize() + "Info", "item_class_name" : item_class_name}
        ofile_path = table_name + "interface.cpp"
        ofile_path_header = table_name + "interface.hpp"
        ofile = open(ofile_path, 'w')
        ofile_header = open(ofile_path_header, 'w')
        interpreter = em.Interpreter(output=ofile, globals=glob)

        try:
            interpreter.file(open("template.txt"))
        except OSError as e:
            ofile.close()
            #os.remove(output_file)
            #raise

        ofile.close()
        interpreter.shutdown()

        interpreter = em.Interpreter(output=ofile_header, globals=glob_header)

        try:
            interpreter.file(open("template_header.txt"))
        except OSError as e:
            ofile.close()
                #os.remove(output_file)
                #raise
    f.close()
    ofile_header.close()

    interpreter.shutdown()
