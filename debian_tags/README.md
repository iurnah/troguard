Author: iurnah,
Email: iurnah@gmail.com

This is the project to study the classification of debian packages,
extract features based on debian pacakges tags and specific field in
control file of the .deb file as a .arff format data set. This is a
particular format to used for weka classification.

The basic idea is to define 10 application types manually based on the
observation of the debian tag database.

--type_def.dat is define the 10 applications with tags from debian database.
--debian_tagdb is from debian website, which is a database with all the
application labled with tags.

--facets_features.cpp, takes the type_def.dat and debian_tagdb as the
commandline input to produce the app_list.dat and named_app.dat file,
named_app.dat file is automatically labeled with types for each application.

One important point here for labelling is we calculate the total number of
apperance of the characteristic facets for the class defined by type_def.dat.

output_arff.cpp is taking the generated files to generate formated data set for
weka. To make the code straightforward, we first generate the header and the
value file seperatly and then combine them as the final data set to feed weka.
Final data set is

debian_pkg_3709.arff

we can use it to test whether it cluster well. 


