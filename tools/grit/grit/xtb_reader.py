# Copyright 2012 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

'''Fast and efficient parser for XTB files.
'''


import re
import sys
import xml.sax
import xml.sax.handler

import grit.node.base
from grit import constants


GRAMMATICAL_GENDER_RE = re.compile(
    (r'^\s*variants\s*{\s*grammatical_gender_variant\s*{\s*'
     r'grammatical_gender_case:\s*(\w+)\s*}\s*}\s*$'))


class XtbContentHandler(xml.sax.handler.ContentHandler):
  '''A content handler that calls a given callback function for each
  translation in the XTB file.
  '''

  def __init__(self, callback, defs=None, debug=False, target_platform=None):
    self.callback = callback
    self.debug = debug
    # 0 if we are not currently parsing a translation, otherwise the message
    # ID of that translation.
    self.current_id = 0
    # None if we are not currently parsing a gendered translation, otherwise a
    # string (expected to be 'OTHER', 'MASCULINE', 'FEMININE', or 'NEUTER').
    # This can be accessed using self.get_effective_gender() to handle the
    # default gender if the value is None.
    self.current_gender = None
    # Empty if we are not currently parsing a translation, otherwise the
    # parts we have for that translation per gender - a dict of gender -> list
    # of tuples - {gender: [(is_placeholder, text)]}
    # This can be accessed using self.get_current_structure() to handle
    # gender and initialization.
    self.current_structure = {}
    # Set to the language ID when we see the <translationbundle> node.
    self.language = ''
    # Keep track of the if block we're inside.  We can't nest ifs.
    self.if_expr = None
    # Root defines to be used with if expr.
    if defs:
      self.defines = defs
    else:
      self.defines = {}
    # Target platform for build.
    if target_platform:
      self.target_platform = target_platform
    else:
      self.target_platform = sys.platform

  def startElement(self, name, attrs):
    if name == 'translation':
      assert self.current_id == 0 and len(self.current_structure) == 0, (
              "Didn't expect a <translation> element here.")
      self.current_id = attrs.getValue('id')
    elif name == 'ph':
      assert self.current_id != 0, "Didn't expect a <ph> element here."
      self.get_current_structure().append((True, attrs.getValue('name')))
    elif name == 'translationbundle':
      self.language = attrs.getValue('lang')
    elif name in ('if', 'then', 'else'):
      assert self.if_expr is None, "Can't nest <if> or use <else> in xtb files"
      self.if_expr = attrs.getValue('expr')
    elif name == 'branch':
      assert self.current_gender is None, "Can't nest <branch> in xtb files"
      variants = attrs.getValue('variants')
      match = GRAMMATICAL_GENDER_RE.match(variants)
      assert match is not None, f'invalid branch variant format: {variants}'
      self.current_gender = match.group(1)

  def endElement(self, name):
    if name == 'translation':
      assert self.current_id != 0

      defs = self.defines
      def pp_ifdef(define):
        return define in defs
      def pp_if(define):
        return define in defs and defs[define]

      # If we're in an if block, only call the callback (add the translation)
      # if the expression is True.
      should_run_callback = True
      if self.if_expr:
        should_run_callback = grit.node.base.Node.EvaluateExpression(
            self.if_expr, self.defines, self.target_platform)
      if should_run_callback:
        self.callback(self.current_id, self.current_structure)

      self.current_id = 0
      self.current_structure = {}
    elif name == 'if':
      assert self.if_expr is not None
      self.if_expr = None
    elif name == 'branch':
      assert self.current_gender is not None, 'unmatched </branch> tag'
      self.current_gender = None

  def shouldRemoveSpaceAfterDeviceType(self, content):
    # If DEVICE_TYPE zh-CN does not end with Chinese characters,
    # this function should not be used,
    # or return False, but it seems that there's no easy way to check
    # the real value of DEVICE_TYPE here
    try:
      previous_item_is_device_type = (len(self.current_structure) > 0
                                        and self.current_structure[-1][0] is True
                                        and self.current_structure[-1][1] == 'DEVICE_TYPE')
      current_not_empty = (len(content) > 1 and content[0] == ' '  and content[1] != ' ')
      return previous_item_is_device_type and current_not_empty
    except Exception:
      return False

  def characters(self, content):
    if self.current_id != 0:
      # We are inside a <translation> node so just add the characters to our
      # structure.
      #
      # This naive way of handling characters is OK because in the XTB format,
      # <ph> nodes are always empty (always <ph name="XXX"/>) and whitespace
      # inside the <translation> node should be preserved.
      # replace double quotation(\u201c, \u201d) with Chinese quotation (\u300c, \u300d)
      if self.language == 'zh-CN':
        if self.shouldRemoveSpaceAfterDeviceType(content):
          content = content[1:]
        self.get_current_structure().append((False,
          content.replace(u'\u201c', u'\u300c')
          .replace(u'\u201d', u'\u300d')
          .replace(u'\u60a8', u'\u4f60')
          .replace(u'\u5e10\u53f7', u'\u8d26\u53f7')))
      else:
        self.get_current_structure().append((False, content))

  # Gets the current gender, or DEFAULT_GENDER if the current gender is None.
  def get_effective_gender(self):
    return self.current_gender or constants.DEFAULT_GENDER

  # Gets the self.current_structure entry for the current gender, including
  # proper initialization.
  def get_current_structure(self):
    g = self.get_effective_gender()
    if g not in self.current_structure:
      self.current_structure[g] = []
    return self.current_structure[g]


class XtbErrorHandler(xml.sax.handler.ErrorHandler):
  def error(self, exception):
    pass

  def fatalError(self, exception):
    raise exception

  def warning(self, exception):
    pass


def Parse(xtb_file, callback_function, defs=None, debug=False,
          target_platform=None):
  '''Parse xtb_file, making a call to callback_function for every translation
  in the XTB file.

  The callback function must have the signature as described below.  The 'parts'
  parameter is a list of tuples (is_placeholder, text).  The 'text' part is
  either the raw text (if is_placeholder is False) or the name of the placeholder
  (if is_placeholder is True).

  Args:
    xtb_file:           open('fr.xtb', 'rb')
    callback_function:  def Callback(msg_id, parts): pass
    defs:               None, or a dictionary of preprocessor definitions.
    debug:              Default False. Set True for verbose debug output.
    target_platform:    None, or a sys.platform-like identifier of the build
                        target platform.

  Return:
    The language of the XTB, e.g. 'fr'
  '''
  # Start by advancing the file pointer past the DOCTYPE thing, as the TC
  # uses a path to the DTD that only works in Unix.
  # TODO(joi) Remove this ugly hack by getting the TC gang to change the
  # XTB files somehow?
  front_of_file = xtb_file.read(1024)
  xtb_file.seek(front_of_file.find(b'<translationbundle'))

  handler = XtbContentHandler(callback=callback_function, defs=defs,
                              debug=debug, target_platform=target_platform)
  xml.sax.parse(xtb_file, handler)
  assert handler.language != ''
  return handler.language
