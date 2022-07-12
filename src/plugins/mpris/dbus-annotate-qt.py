#!/usr/bin/env python3

import argparse
from xml.etree import ElementTree
from xml.sax.saxutils import escape


def parse_arguments():
    parser = argparse.ArgumentParser()

    parser.add_argument('xml_files', type=str, nargs='+', help='List of XML DBUS interface description files')

    return parser.parse_args()


def annotate_property(node, mapping) -> bool:
    if 'type' not in node.attrib:
        return False

    prop_type = node.attrib['type']
    if prop_type not in mapping:
        return False

    annotation_name = 'org.qtproject.QtDBus.QtTypeName'
    is_annotated = any([
        ('name' in annotation.attrib and annotation.attrib['name'] == annotation_name)
        for annotation in node.findall('annotation')
    ])

    if not is_annotated:
        qt_type = escape(mapping[prop_type])
        annotation = f'annotation name="org.qtproject.QtDBus.QtTypeName" value="{qt_type}"'
        ElementTree.SubElement(node, annotation)
        print(f'Annotated {prop_type} as {qt_type}')
        return True

    return False


def annotate_arg_container(node, mapping) -> bool:
    in_counter = 0
    out_counter = 0

    tree_modified = False

    for subnode in node.findall('arg'):
        if 'type' not in subnode.attrib:
            continue

        prop_type = subnode.attrib['type']
        if prop_type not in mapping:
            continue

        direction = 'out'
        if 'direction' in subnode.attrib:
            direction = subnode.attrib['direction'].lower()

        if direction == 'in':
            in_counter += 1
        elif direction == 'out':
            out_counter += 1
        else:
            print(f'Unknown direction {direction}')

        qt_type = escape(mapping[prop_type])
        counter = out_counter if direction == 'out' else in_counter
        annotation_name = f'org.qtproject.QtDBus.QtTypeName.{direction.capitalize()}{counter - 1}'

        is_annotated = any([
            ('name' in annotation.attrib and annotation.attrib['name'] == annotation_name)
            for annotation in node.findall('annotation')
        ])

        if not is_annotated:
            annotation = f'annotation name="{annotation_name}" value="{qt_type}"'

            ElementTree.SubElement(node, annotation)
            print(f'Annotated arg {direction.capitalize()}{counter} {prop_type} as {qt_type}')
            tree_modified = True

    return tree_modified


def main() -> int:
    args = parse_arguments()

    mapping = {
        'a{sv}': 'QVariantMap',
        'aa{sv}': 'QVector<QVariantList>',
        '(oss)': 'MprisPlaylist',
        'a(oss)': 'MprisPlaylistList',
        '(b(oss))': 'MprisMaybePlaylist',
    }

    for xml_file in args.xml_files:
        tree = ElementTree.parse(xml_file)
        tree_modified = False

        root = tree.getroot()

        if not len(root) > 0 and not root[0].tag == 'interface':
            print('XML file is missing interface node')
            return 1

        for node in root[0]:
            is_property = node.tag == 'property'
            is_arg_container = node.tag in ['method', 'signal']

            if is_property:
                tree_modified |= annotate_property(node, mapping)
            elif is_arg_container:
                tree_modified |= annotate_arg_container(node, mapping)

        if tree_modified:
            tree.write(xml_file)

    return 0


if __name__ == "__main__":
    exit(main())
