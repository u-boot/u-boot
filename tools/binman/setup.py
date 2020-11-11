# SPDX-License-Identifier: GPL-2.0+

from distutils.core import setup
setup(name='binman',
      version='1.0',
      license='GPL-2.0+',
      scripts=['binman'],
      packages=['binman', 'binman.etype'],
      package_dir={'binman': ''},
      package_data={'binman': ['README', 'README.entries']},
      classifiers=['Environment :: Console',
                   'Topic :: Software Development :: Embedded Systems'])
