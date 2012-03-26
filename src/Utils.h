#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>
using std::string;

bool   FileExists     (string  filename);
string RemoveExtension(string &filename);

#endif /* __UTILS_H__ */
