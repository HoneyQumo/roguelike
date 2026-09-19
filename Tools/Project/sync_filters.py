"""Приводит дерево Visual Studio в соответствие со сборкой.

MSBuild собирает по .vcxproj, а Solution Explorer рисует дерево по .vcxproj.filters.
Файл легко добавить в первый и забыть про второй - тогда он собирается, но в дереве
его нет. Этот скрипт пересобирает .filters по .vcxproj, раскладывая файлы по папкам.

Идентификаторы папок берутся из старого файла, чтобы VS не считала папки новыми.
"""

import os
import re
import uuid

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..', '..'))

PROJECTS = [
    os.path.join(ROOT, 'Roguelike', 'Roguelike.vcxproj'),
    os.path.join(ROOT, 'XYZEngine', 'XYZEngine.vcxproj'),
]

HEADER = ('<?xml version="1.0" encoding="utf-8"?>\r\n'
          '<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">\r\n')


def Read(path):
    with open(path, 'rb') as file:
        return file.read().decode('utf-8-sig')


def Entries(text, tag):
    return re.findall(r'<' + tag + r'\s+Include="([^"]+)"', text)


def FolderOf(include):
    parts = include.split('\\')

    return parts[0] if len(parts) > 1 else ''


def KnownIds(path):
    """Старые идентификаторы папок: если их менять, VS покажет папки как новые."""
    if not os.path.exists(path):
        return {}

    text = Read(path)
    found = re.findall(r'<Filter Include="([^"]+)">\s*<UniqueIdentifier>\{([^}]+)\}</UniqueIdentifier>', text)

    return {name: identifier for name, identifier in found}


def IdFor(folder, known):
    if folder in known:
        return known[folder]

    # Одна и та же папка обязана получить один и тот же id при каждом прогоне,
    # иначе файл меняется без причины на каждой сборке.
    return str(uuid.uuid5(uuid.NAMESPACE_URL, 'roguelike/filters/' + folder))


def Build(project):
    filters = project + '.filters'
    text = Read(project)
    known = KnownIds(filters)

    groups = {}
    for tag in ('ClCompile', 'ClInclude'):
        for include in Entries(text, tag):
            groups.setdefault(tag, []).append(include)

    folders = sorted({FolderOf(include) for tag in groups for include in groups[tag]} - {''})

    out = [HEADER, '  <ItemGroup>\r\n']
    for folder in folders:
        out.append('    <Filter Include="%s">\r\n' % folder)
        out.append('      <UniqueIdentifier>{%s}</UniqueIdentifier>\r\n' % IdFor(folder, known))
        out.append('    </Filter>\r\n')
    out.append('  </ItemGroup>\r\n')

    for tag in ('ClCompile', 'ClInclude'):
        if tag not in groups:
            continue

        out.append('  <ItemGroup>\r\n')
        for include in sorted(groups[tag], key=lambda name: name.lower()):
            folder = FolderOf(include)
            if folder:
                out.append('    <%s Include="%s">\r\n' % (tag, include))
                out.append('      <Filter>%s</Filter>\r\n' % folder)
                out.append('    </%s>\r\n' % tag)
            else:
                out.append('    <%s Include="%s" />\r\n' % (tag, include))
        out.append('  </ItemGroup>\r\n')

    out.append('</Project>')

    with open(filters, 'wb') as file:
        file.write(b'\xef\xbb\xbf' + ''.join(out).encode('utf-8'))

    total = sum(len(names) for names in groups.values())
    print('synced %-24s %d files in %d folders' % (os.path.basename(filters), total, len(folders)))


if __name__ == '__main__':
    for project in PROJECTS:
        if os.path.exists(project):
            Build(project)
