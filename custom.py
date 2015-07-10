import os
HOME       = os.environ['HOME']
BOOST_ROOT = os.environ.get('BOOST_ROOT',"/usr")

# detect os
os.system("uname > /tmp/platform")
OS = open("/tmp/platform","r").readlines()[0].strip()

# 
def parse(text):
    return filter(lambda s: len(s) > 0, 
                  [s.strip().replace("${HOME}",HOME).replace("${BOOST_ROOT}",BOOST_ROOT) for s in text.split()])

class Target(object):

    def __init__(self, parent=None, name=None, subtarget=None):
        self.name          = name
        self.parent        = parent
        self.subtarget     = subtarget
        self.kind          = "program" # "shlib", "stlib"
        self.use           = []
        self.source        = []
        self.includes      = []
        self.linkflags     = []
        self.libpath       = []
        self.lib           = []
        self.cxxflags      = []
        self.cflags        = []
        self.frameworkpath = []
        self.framework     = []
        self.subtargets    = {}

    def Kind(self, kind):
        self.kind = kind
        return self

    def __append__(self, field_name, mode, input_list):
        if mode == "set":
            self.__dict__[field_name] = input_list
        else:
            self.__dict__[field_name] += input_list
        return self

    def Includes(self, lst, mode="append"):
        return self.__append__("includes", mode, lst)

    def Use(self, lst, mode="append"):
        return self.__append__("use", mode, lst)

    def Source(self, lst, mode="append"):
        return self.__append__("source", mode, lst)

    def Libpath(self, lst, mode="append"):
        return self.__append__("libpath", mode, lst)

    def Lib(self, lst, mode="append"):
        return self.__append__("lib", mode, lst)

    def Linkflags(self, lst, mode="append"):
        return self.__append__("linkflags", mode, lst)

    def Cxxflags(self, lst, mode="append"):
        return self.__append__("cxxflags", mode, lst)

    def Cflags(self, lst, mode="append"):
        return self.__append__("cflags", mode, lst)

    def Frameworkpath(self, lst, mode="append"):
        return self.__append__("frameworkpath", mode, lst)
  
    def Framework(self, lst, mode="append"):
        return self.__append__("framework", mode, lst)

    def Subtarget(self, key):
        if not self.subtargets.get(key,None):
            self.subtargets[key] = Target(self, self.name, subtarget=key)
        return self.subtargets[key]

    def SubtargetPath(self):
        result = []
        node = self
        while node.subtarget:
            result.insert(0, node.subtarget)
            node = node.parent
        return result

    def MainTarget(self):
        node = self
        while node.subtarget:
            node = node.parent
        return node

    def SubtargetUsingPath(self, path):
        result = self
        for i in xrange(len(path)):
            candidate = result.subtargets.get(path[i], None)
            if candidate:
                result = candidate
            else:
                break
        return result

    def GetValue(self, property_name):
        result = []

        # get value from this
        path = self.SubtargetPath()

        pairs = [ (self, self.MainTarget()) ]
        while len(pairs):
            subtarget, target = pairs.pop()
            # print "subtarget: ", subtarget.name, "_", subtarget.subtarget
            # print "target:", target.name, "_",  target.subtarget
            while True:
                # print subtarget.subtarget
                result += subtarget.__dict__[property_name]
                if subtarget == target:
                    break
                subtarget = subtarget.parent

            next_main = target.parent

            if next_main != None:
                next_sub = next_main.SubtargetUsingPath(path)
                pairs.append( (next_sub, next_main) )


        # while 
        # result = self.__dict__[property_name]
        # p = self.parent
        # while p != None:
        #     result += p.__dict__[property_name]
        #     p = p.parent

        def uniq(seq):
            seen = set()
            seen_add = seen.add
            return [ x for x in seq if x not in seen and not seen_add(x)]

        return uniq(result)
  
    def Pop(self):
        return self.parent
