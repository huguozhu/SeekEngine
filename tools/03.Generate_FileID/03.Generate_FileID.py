import os

MACRO_BEFORE = '///!>>> DVF_MACRO_FILE_UID'
MACRO_AFTER1 = '#define DVF_MACRO_FILE_UID '
MACRO_AFTER2 = '#undef DVF_MACRO_FILE_UID'
MACRO_COMMEND = "     // this code is auto generated, don't touch it!!!"

dvf_path = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
walk_path = [
    os.path.join(dvf_path, 'engine'),
    os.path.join(dvf_path, 'wrapper'),
    os.path.join(dvf_path, 'samples'),
    os.path.join(dvf_path, 'tools'),
]
# print("dvf_path", dvf_path)

# Get all filepaths need to process
filepaths = []
for path in walk_path:
    for root, _, files in os.walk(path):
        for file in files:
            if file.endswith('.cpp') or file.endswith('.h') or file.endswith('.mm') or file.endswith('.m') or file.endswith('.hpp'):
                filepaths.append(os.path.join(path, root, file))

# Valid file_uid (1 ~ )
file_uid_valid = [i for i in range(1, 1024)]
file_uid_map = {}



class SourceCodeFile:
    def __init__(self, filepath):
        self.source_code_lines = []
        self.linenum_before = -1
        self.linenum_after1 = -1
        self.linenum_after2 = -1
        self.filepath = filepath

    def load(self):
        with open(self.filepath, 'r') as f:
            self.source_code_lines = f.readlines()
        self.linenum_before = -1
        self.linenum_after1 = -1
        self.linenum_after2 = -1
        for linenum, line in enumerate(self.source_code_lines):
            line = line.strip()
            if line.startswith(MACRO_BEFORE):
                self.linenum_before = linenum
            elif line.startswith(MACRO_AFTER1):
                self.linenum_after1 = linenum
            elif line.startswith(MACRO_AFTER2):
                self.linenum_after2 = linenum

    def reload(self):
        with open(self.filepath, 'w') as f:
            f.writelines(self.source_code_lines)
        self.load()

    def delete_linenum(self, linenum):
        if linenum >= 0:
            del self.source_code_lines[linenum]
            self.reload()

    def read_all(self):
        self.load()

        if self.linenum_before >= 0 and (self.linenum_after1 >= 0 or self.linenum_after2 >= 0):
            self.delete_linenum(self.linenum_after1)
            self.delete_linenum(self.linenum_after2)

        if self.linenum_after1 >= 0:
            if self.linenum_after2 < 0:
                self.source_code_lines.append('\n' + MACRO_AFTER2 + MACRO_COMMEND + '\n')
                self.reload()

            file_uid = int(self.source_code_lines[self.linenum_after1][len(MACRO_AFTER1):len(MACRO_AFTER1)+5].strip())
            if file_uid not in file_uid_valid or file_uid in file_uid_map:
                if file_uid in file_uid_map:
                    print('dup uid', self.filepath, file_uid_map[file_uid], 'auto change')
                self.source_code_lines[self.linenum_after1] = MACRO_BEFORE + '\n'
                self.reload()
                self.delete_linenum(self.linenum_after2)
            else:
                file_uid_map[file_uid] = self.filepath
                del file_uid_valid[file_uid_valid.index(file_uid)]

    def replace_all(self):
        self.load()
        if self.linenum_before >= 0:
            self.source_code_lines[self.linenum_before] = MACRO_AFTER1 + str(file_uid_valid[0]) + MACRO_COMMEND + '\n'
            self.source_code_lines.append('\n' + MACRO_AFTER2 + MACRO_COMMEND + '\n')
            file_uid_map[file_uid_valid[0]] = self.filepath
            del file_uid_valid[0]
            self.reload()

# Firstly, read all files
for _filepath in filepaths:
    scf = SourceCodeFile(_filepath)
    scf.read_all()
# Then, replace all files
for _filepath in filepaths:
    scf = SourceCodeFile(_filepath)
    scf.replace_all()

output_file_uid_lines = []
output_file_uid_lines.append(MACRO_COMMEND.strip() + '\n')
output_file_uid_lines.append('#pragma once\n')
output_file_uid_lines.append('\nstatic char const * const __file_uid_to_path[] = {\n')
output_file_uid_lines.append('    "",\n')
for i in range(1, 1024):
    if i in file_uid_map.keys():
        output_file_uid_lines.append('    /* %4d */ "' % i + file_uid_map[i][len(dvf_path)+1:] + '",\n')
    else:
        output_file_uid_lines.append('    /* %4d */ "' % i + '",\n')
output_file_uid_lines.append('};\n')


# error code
header_path1 = os.path.join(dvf_path,             'common', 'include', 'dvf', 'dvf.h')
header_path2 = os.path.join(dvf_path, '..', '..', 'common', 'include', 'dvf', 'dvf.h')
if os.path.exists(header_path1):
    with open(header_path1, 'r') as f:
        header_content = f.readlines()
elif os.path.exists(header_path2):
    with open(header_path2, 'r') as f:
        header_content = f.readlines()
else:
    print('not found dvf.h')
    exit

errors = {}
for line in header_content:
    line = line.strip()
    if line.startswith('DVF_ERR'):
        line_split = line[:-1].split('=')
        if len(line_split) > 1:
            id = int(line_split[1].strip())
            errors[id] = line_split[0].strip()

output_file_uid_lines.append('\n')
output_file_uid_lines.append('\n')
output_file_uid_lines.append('\n')
output_file_uid_lines.append('\n')
output_file_uid_lines.append('static char const * const __error_code_str[] = {\n')
output_file_uid_lines.append('    "",\n')
for i in range(1, 256):
    if i in errors.keys():
        output_file_uid_lines.append('    /* %3d */ "' % i + errors[i] + '",\n')
    else:
        output_file_uid_lines.append('    /* %3d */ "' % i + '",\n')
output_file_uid_lines.append('};\n')



with open(os.path.join(dvf_path, 'engine', 'util', 'file_uid.h'), 'w') as f:
    f.writelines(output_file_uid_lines)
