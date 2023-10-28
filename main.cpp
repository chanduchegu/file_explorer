#include<iostream>
#include<dirent.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<stdbool.h>
#include<ctime>
#include <fstream>
#include <bitset>
#include<bits/stdc++.h>
#include<grp.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <pwd.h>
#include<signal.h>
#include <sys/ioctl.h>
using namespace std;
stack<string>prev_visited,next_visited;
int x_coordinate,y_coordinate;
int window_size;
int start_index=0,end_index;
string current_working_directory="";
struct termios orig_termios;
class contents
{
    public:
    string name;
    string permissions;
    string size;
    string lastModified;
    string ownership_user;
    string ownership_group;
    int isDir;
    string path;
    
    contents(string n,string s,string o_u,string o_g,string p,string l,int dir,string pa)
    {
        name=n;
        permissions=p;
        size=s;
        lastModified=l;
        ownership_user=o_u;
        ownership_group=o_g;
        isDir=dir;
        path=pa;
    }
};
vector<string>filenames;
vector<contents>fileInfo;
bool normalMode;
string cmd_str="";
void getWinSize()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        window_size=w.ws_row;
}
string getPermissions(char*name)
{
    struct stat fileStatus;
    if(stat(name,&fileStatus)<0)
    {
        cout<<"file permissions can't be found"<<endl;
        return "";
    }
    string permissions="";
    permissions+=S_ISDIR(fileStatus.st_mode) ? "d" : "-";
    permissions+=fileStatus.st_mode & S_IRUSR ? "r" : "-";
    permissions+=fileStatus.st_mode & S_IWUSR ? "w" : "-";
    permissions+=fileStatus.st_mode & S_IXUSR ? "x" : "-";
    permissions+=fileStatus.st_mode & S_IRGRP ? "r" : "-";
    permissions+=fileStatus.st_mode & S_IWGRP ? "w" : "-";
    permissions+=fileStatus.st_mode & S_IXGRP ? "x" : "-";
    permissions+=fileStatus.st_mode & S_IROTH ? "r" : "-";
    permissions+=fileStatus.st_mode & S_IWOTH ? "w" : "-";
    permissions+=fileStatus.st_mode & S_IXOTH ? "x" : "-";
    return permissions;
    
}
string getLastModified(char*name)
{   
    struct stat file_status;
    if(stat(name,&file_status)<0)
    {
        return "";
    }
    char mtime[80];
    time_t t = file_status.st_mtime;
    struct tm lt;
    localtime_r(&t, &lt);
    strftime(mtime, sizeof mtime, "%b %d %T", &lt);
    return mtime;
}
string getFileSize(char*name)
{
    struct stat file_status;
    if(stat(name,&file_status)<0)
    {
        // cout<<"chadfsd"<<endl;
        return "";
    }
    long long size=stoi(to_string(file_status.st_size));
    //converting the file from integer to string
    // cout<<size<<"dsf"<<endl;
    if(size<1024)
    {
        string s=to_string(size);
        s+="B";
        return s;
    }
    else if(size<1048576)
    {
        size=size/1024;
        string s=to_string(size);
        s+="KB";
        return s;
    }
    else if(size<1073741824)
    {
        size=size/1048576;
        string s=to_string(size);
        s+="MB";
        return s;
    }
    else if(size<1099511627776)
    {
        size=size/1073741824;
        string s=to_string(size);
        s+="MB";
        return s;
    }
    return "";
}
string getOwner_user(char*name)
{
    return getlogin();
}
string getOwner_group(char*name)
{
    struct stat file_status;
    if(stat(name,&file_status)<0)
    {
        return "";
    }
    return getgrgid(file_status.st_gid)->gr_name;
}
bool getFiletype(char*name)
{
    struct stat file_status;
    if(stat(name,&file_status)<0)
    {
        return "";
    }
    return S_ISDIR(file_status.st_mode);
}
string getFileNameFromPath(string path)
{
    int n=path.size();
    string name="";
    for(int i=n-1; i>0; i--)
    {
        if(path[i]=='/')
            break;
        else
            name=path[i]+name;
    }
    return name;
}
string getDirectoryPath(string path)
{
    int n=path.size();
    for(int i=n-1; i>=0; i--)
    {
        if(path[i]=='/')
        return path.substr(0,i);
    }
    return "";
}
void updateCursor()
{
    y_coordinate=0;
    start_index=0;
    end_index=min(window_size-4,(int)fileInfo.size()-1);
}
void printContents()
{
    cout << "\033[2J\033[1;1H";
    getWinSize();
    for(int i=start_index; i<=end_index; i++)
    {
        auto eachFile=fileInfo[i];
        if(i==y_coordinate+start_index)
            cout<<">>>"<<"\t";
        else
            cout<<"   "<<"\t";
        // cout<<i<<" ";    
        cout<<left<<setw(40)<<eachFile.name<<"\t";
        cout<<left<<setw(10)<<eachFile.size<<"\t";
        cout<<left<<setw(15)<<eachFile.ownership_user<<"\t";
        cout<<left<<setw(15)<<eachFile.ownership_group<<"\t";
        cout<<left<<setw(10)<<eachFile.permissions<<"\t";
        cout<<left<<setw(20)<<eachFile.lastModified<<"\t";
        cout<<left<<setw(5)<<eachFile.isDir<<endl;
    }
    for(int i=end_index-start_index+2; i<window_size-2; i++)
        cout<<endl;
    if(normalMode==true)
        cout<<"NormalMode :"<<current_working_directory<<endl;
    else
        cout<<"Command Mode $ "<<cmd_str<<flush;
}
string getParentDirectory(string cwd)
{
    int n=cwd.size();
    string parent_dir;
    bool flag=false;
    for(int i=n-1; i>0; i--)
    {
        if(cwd[i]=='/')
        {
             parent_dir=cwd.substr(0,i);
            flag=true;
            break;
        }
    }
    
    if(flag==true)
    {   
        return parent_dir;
        // prev_visited.push(current_working_directory);
        // current_working_directory=parent_dir;
        // getContents(current_working_directory);
    }
    else
        return cwd;
}
string getHomeDirectory()
{
    int userId=getuid();
    passwd *userpwsd=getpwuid(userId);
    string home_dir=userpwsd->pw_dir;
    return home_dir;
}
bool getContents(string cwd)
{
    //opening the directory and getting the contents
    
    const char*current_dir=cwd.c_str();
    // cout<<"dasf"<<current_dir<<endl;
    DIR *dir= opendir(current_dir);
    if(dir==NULL)
    {
        cout<<"directory cannot be opened"<<endl;
        closedir(dir);
        return false;
    }
    filenames.clear();
    if(dir!=NULL)
    {
        for(struct dirent*d=readdir(dir); d!=NULL; d=readdir(dir))
        {
            filenames.push_back(d->d_name)  ;
        }
        closedir(dir);
    }
    sort(filenames.begin(),filenames.end());
    //to clear the screen and printing the contents from top
    fileInfo.clear();
    for(auto eachFile:filenames)
    {
        string path;
        if(eachFile==".")
            path=cwd;
        else if(eachFile=="..")
        {
            path=getParentDirectory(current_working_directory);
        }
        else
         path=cwd+'/'+eachFile;
        char *name=const_cast<char*>(path.c_str());
        string lastModified=getLastModified(name);
        string size=getFileSize(name);
        string permi=getPermissions(name);
        string owner_user=getOwner_user(name);
        string owner_group=getOwner_group(name);
        bool isDir=getFiletype(name);
        contents *x=new contents(eachFile,size,owner_user,owner_group,permi,lastModified,isDir,path);
        fileInfo.push_back(*x);
    }
    updateCursor();
    printContents();
    // closedir(dir);
    return true;
}
void deleteRightStack()
{
    while(!next_visited.empty())
    {
        next_visited.pop();
    }
}
void upperArrow()
{
    if(y_coordinate>0)
        y_coordinate--;
    else 
    {
        if(start_index>0)
        {
            start_index-=1;
            end_index-=1;
        }
        
    }
    printContents();
}
void lowerArrow()
{   
    if(y_coordinate<min(window_size-4,(int)fileInfo.size()-1))
        y_coordinate++;
    else 
    {
        if(end_index<fileInfo.size()-1)
        {
            start_index+=1;
            end_index+=1;
        }
    }
    printContents();
}
void backSpace()
{

    string cwd=current_working_directory;
    string parent_dir=getParentDirectory(cwd);
    if(cwd!=parent_dir)
    {
           prev_visited.push(current_working_directory);
           current_working_directory=parent_dir;
           updateCursor();
            deleteRightStack();
           getContents(current_working_directory);
    }
}
void home()
{
    //to get userid of the calling process
    int userId=getuid();
    // The __getpwuid()__ function returns a pointer to a structure containing the broken-out fields of the record in the password database that matches the user ID uid.
    passwd *userpwsd=getpwuid(userId);
    //pw_dir contains the path for home directory
    string home_dir=userpwsd->pw_dir;
    if(current_working_directory!=home_dir)
    {
        prev_visited.push(current_working_directory);
        current_working_directory=home_dir;
        updateCursor();
        deleteRightStack();
        getContents(current_working_directory);
    }
    

}
void leftArrow()
{
    // cout<<"ad"<<endl;
    if(!prev_visited.empty())
    {
        next_visited.push(current_working_directory);
        current_working_directory=prev_visited.top();
        prev_visited.pop();
        updateCursor();
        getContents(current_working_directory);
    }
    
}
void rightArrow()
{
    if(!next_visited.empty())
    {
        prev_visited.push(current_working_directory);
        current_working_directory=next_visited.top();
        next_visited.pop();
        updateCursor();
        getContents(current_working_directory);
    }
}
void enter()
{
    // cout<<"hi"<<endl;
    int offset=y_coordinate+start_index;
    if(fileInfo[offset].isDir)
    {
        prev_visited.push(current_working_directory);
        current_working_directory=fileInfo[offset].path;
        updateCursor();
        deleteRightStack();
        getContents(current_working_directory);
    }
    else{
        int pid=fork();
        if(pid==0)
        {
            string p=fileInfo[offset].path;
            const char*path=p.c_str();
            execl("/usr/bin/xdg-open","/usr/bin/xdg-open",path,(char*)NULL);
            exit(1);
        }
    }
}
void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
void enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
void enableRawMode1() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO );
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
bool search(string curr_dir,string key)
{
    // cout<<"sadf"<<endl;
    // cout<<"hi"<<curr_dir<<endl;
    DIR*dir=opendir(curr_dir.c_str());
    if(dir==NULL)
    {
        // cout<<curr_dir<<" "<<"hello"<<endl;
        return false; 
    }
    if(dir!=NULL)
    {
        for(struct dirent*d=readdir(dir); d!=NULL; d=readdir(dir))
        {
            // cout<<d->d_name<<" "<<key<<endl;
            string var=d->d_name;
            if(var==key)
            {
                // cout<<"hello"<<endl;
                return true;
            }
                
            if(string(d->d_name)=="."|| string(d->d_name)=="..")
                continue;
            string next_dir=curr_dir+'/'+d->d_name;
            char*path=const_cast<char*>(next_dir.c_str());
            if(getFiletype(path))
            {
                // cout<<next_dir<<"  dsfa"<<endl;

                if(search(next_dir,key))
                {
                    return true;
                }
                else
                continue;
            }   
            else
                continue; 
        }
        
    }
    return false;
}
bool copyFile(string sourcePath,string destPath)
{
    
    char *sourceFile=const_cast<char*>(sourcePath.c_str());
    char *destFile=const_cast<char*>(destPath.c_str());
    FILE*fptr1=fopen(sourceFile,"rb");
    FILE*fptr2=fopen(destFile,"wb");
    if(fptr1==NULL)
    {
        cout<<"cannot open file "<<fptr1<<endl;
        return false;
    }
    if(fptr2==NULL)
    {
        cout<<"cannot open filea "<<fptr2<<endl;
        return false;
    }
    //BUFSIZ default is 8192 bytes
    char buf[BUFSIZ];
    size_t size;
     while (size = fread(buf, 1, BUFSIZ, fptr1)) {
        fwrite(buf, 1, size, fptr2);
    }
    struct stat w;
	stat(sourceFile, &w);
	chown(destFile, w.st_uid, w.st_gid);
	chmod(destFile, w.st_mode);
    fclose(fptr1);
    fclose(fptr2);
    return true;
}
bool copyDir(string sourcePath,string dirName,string destination)
{
    // cout<<sourcePath<<" "<<dirName<<" "<<destination<<endl;
    string destPath=destination+'/'+dirName;
    char*path=const_cast<char*>(destPath.c_str());
    int flag=mkdir(path,0777);
    if(flag)
    {
        cout<<"unable to create directory"<<endl;
        return false;
    }
    DIR*dir=opendir(sourcePath.c_str());
    if(dir==NULL)
    {
        // cout<<"copy failed"<<endl;
        return false; 
    }
    if(dir!=NULL)
    {
        for(struct dirent*d=readdir(dir); d!=NULL; d=readdir(dir))
        {
            // cout<<d->d_name<<endl;
            string s=string(d->d_name);
            if(s=="." || s=="..")
            {
                // cout<<"hello"<<endl;
                continue;
            }        
            else
            {
                string next_dir=sourcePath+'/'+d->d_name;
                char*path1=const_cast<char*>(next_dir.c_str());
                // cout<<d->d_name<<endl;
                if(getFiletype(path1))
                {
                    if(copyDir(sourcePath+'/'+d->d_name,d->d_name,destination+'/'+dirName)==false)
                        return false;
                }   
                else
                {
                    // cout<<"file copy"<<sourcePath+'/'+d->d_name<<" "<<destination+'/'+dirName+'/'+d->d_name<<endl;
                    if(copyFile(sourcePath+'/'+d->d_name,destination+'/'+dirName+'/'+d->d_name)==false)
                        return false;
                }
            }    
             
        }
        
    }
    return true;

}
void copyCommand(vector<string>cmd)
{
     int n=cmd.size();
     string dest=cmd[n-1];
     for(int i=1; i<n-1; i++)
     {
        string sourcePath=cmd[i];
        char *name=const_cast<char*>(sourcePath.c_str());
        if(getFiletype(name))
        {
            // copy directory
            int n=sourcePath.size();
            string directoryName="";
            for(int i=n-1;i>=0; i--)
            {
                if(sourcePath[i]!='/')
                {
                    directoryName=sourcePath[i]+directoryName;
                    
                }
                else if(sourcePath[i]=='/')
                {
                    break;
                }
            }
            // if(search(sourcePat))
            if(copyDir(sourcePath,directoryName,dest))
                cout<<"copied successfully!"<<endl;
            else    
                cout<<"copied failed"<<endl;
            
        }
        else
        {
            // cout<<"copying files"<<endl;
            int n=sourcePath.size();
            string fileName="";
            for(int i=n-1;i>=0; i--)
            {
                if(sourcePath[i]!='/')
                {
                    fileName=sourcePath[i]+fileName;
                    
                }
                else if(sourcePath[i]=='/')
                {
                    break;
                }
            }
            // // cout<<fileName<<" "<<dest<<endl;
            // if(search(dest,fileName)==true)
            // {
            //     // cout<<"hello"<<endl;
            //     int r=fileName.size();
            //     for(int i=r-1; i>=0; i--)
            //     {
            //         if(fileName[i]=='.')
            //         {
            //             fileName=fileName.substr(0,i)+"_copy"+fileName.substr(i,r-i);
            //         }
            //         else
            //             continue;
            //     }
            // }
            // cout<<sourcePath<<" "<<fileName<<endl;
            if(copyFile(sourcePath,dest+'/'+fileName))
                cout<<"copied successfully"<<endl;
            else
                cout<<"copy failed"<<endl;
        }
        
     }   
}
bool deleteFile(string fileName)
{
    // string fileName=cmd[1];
    // cout<<"helo"<<endl;
    char *name=const_cast<char*>(fileName.c_str());
    if(remove(name)==0)
        return true;
    else
        return false;
}
bool deleteDir(string directory)
{
    DIR*dir=opendir(directory.c_str());
    if(dir==NULL)
    {
        return false; 
    }
    if(dir!=NULL)
    {
        for(struct dirent*d=readdir(dir); d!=NULL; d=readdir(dir))
        {
            if(string(d->d_name)=="."|| string(d->d_name)=="..")
                continue;
            string next=directory+'/'+d->d_name;
            char*path=const_cast<char*>(next.c_str());
            if(getFiletype(path))
            {
                if(deleteDir(next)==false)
                    return false;
            }   
            else
                if(deleteFile(next)==false)
                    return false;
        }
        
    }
    if(remove(directory.c_str())==0)
        return true;
    else
        return false;
}
bool moveCommand(vector<string>cmd)
{
    int n=cmd.size();
     string dest=cmd[n-1];
    //  const char*destDirPath=(const char*)destDir.c_str();
    //  bool flag=false;
     for(int i=1; i<n-1; i++)
     {
        string sourcePath=cmd[i];
        char *name=const_cast<char*>(sourcePath.c_str());
        if(getFiletype(name))
        {
            // copy directory
            // cout<<"copying directory"<<endl;
            int n=sourcePath.size();
            string directoryName="";
            for(int i=n-1;i>=0; i--)
            {
                if(sourcePath[i]!='/')
                {
                    directoryName=sourcePath[i]+directoryName;
                    
                }
                else if(sourcePath[i]=='/')
                {
                    break;
                }
            }
            if(copyDir(sourcePath,directoryName,dest)==false)
                return false;
            if(deleteDir(sourcePath)==false)
                return false;
        }
        else
        {
            // cout<<"moving files"<<endl;
            int n=sourcePath.size();
            string fileName="";
            for(int i=n-1;i>=0; i--)
            {
                if(sourcePath[i]!='/')
                {
                    fileName=sourcePath[i]+fileName;
                    
                }
                else if(sourcePath[i]=='/')
                {
                    break;
                }
            }
            // cout<<fileName<<endl;
            if(copyFile(sourcePath,dest+'/'+fileName)==false)
                return false;
            if(remove(name)!=0)
                return false;
        }
        
     }
     return true;
}
bool rename(vector<string>cmd)
{
    string oldfilePath=cmd[1];
    string newFilePath=cmd[2];
    //absolute path is given
    cout<<oldfilePath<<" "<<newFilePath<<endl;
    if(rename(oldfilePath.c_str(),newFilePath.c_str())==0)
        return true;
    else 
        return false;    
}
bool Goto(string dir)
{
    
    // cout<<dir<<endl;
    // cout<<"hello"<<endl;
    if(getContents(dir))
    {
        prev_visited.push(current_working_directory);
        current_working_directory=dir;
        while(!next_visited.empty())
        {
            next_visited.pop();
        }
        return true;
    }

    // else
        // current_working_directory=di;
    
    return false;   
}
bool createFile(string fileName,string destPath)
{
    string filePath=destPath+'/'+fileName;
    char*path=const_cast<char*>(filePath.c_str());
    FILE*fptr=fopen(path,"w");
    if(fptr==NULL)
    {
        return false;
    }
    else
        return true;
        
}
bool createDir(string dirName,string destPath)
{
    string dirPath=destPath+'/'+dirName;
    char*path=const_cast<char*>(dirPath.c_str());
    int flag=mkdir(path,0777);
    if(!flag)
    {
        return true;
    }
    else
        return false;  
}
string convert_path_to_absolute(string path)
{
    int n=path.size();
    //absolute path
    if(path[0]=='/')
        return path;
    if(path[0]=='.' and path[1]=='.')
        return getParentDirectory(current_working_directory)+path.substr(2,n);
    if(path[0]=='.')
        return current_working_directory+path.substr(1,n);
    if(path[0]=='~')
        return getHomeDirectory()+path.substr(1,n);
    else
        return current_working_directory+'/'+path;

}
void command(vector<string>cmd)
{
    int n=cmd.size();
    // if(cmd[0]!="search")
    // {
    //     int r=1;
    //     if(cmd[0]=="create_file" or cmd[0]=="create_dir")
    //         r=2;
    //     for(int i=r; i<n; i++)
    //     {
    //         string name=getFileNameFromPath(cmd[i]);
    //         string path=getDirectoryPath(cmd[i]);
    //         cout<<name<<" "<<path<<endl;
    //         if(search(path,name)==false)
    //         {
    //             cout<<"INVALID SOURCE OR DESTINATION PATHS"<<endl;
    //             return ;
    //         }
    //     }
    // }
    if(cmd.size()==0)
    {
        cout<<"command not entered"<<endl;
        return ;
    }
    else if(cmd[0]=="copy")
    {
        if(cmd.size()<=2)
        {
            cout<<"INVALID ARGUMENTS"<<endl;
            return ;
        }
        copyCommand(cmd);
    }
    else if(cmd[0]=="move")
    {
        if(cmd.size()<=2)
        {
            cout<<"INVALID ARGUMENTS"<<endl;
            return ;
        }
        moveCommand(cmd);
    }
    else if(cmd[0]=="delete_file")
    {
        if(cmd.size()<=1)
        {
            cout<<"INVALID ARGUMENTS"<<endl;
            return ;
        }
        if(deleteFile(cmd[1]))
            cout<<"file deleted successfully"<<endl;
        else   
            cout<<"unable to delete a file"<<endl;
    }
    else if(cmd[0]=="delete_dir")
    {
        if(cmd.size()<=1)
        {
            cout<<"INVALID ARGUMENTS"<<endl;
            return ;
        }
        if(deleteDir(cmd[1]))
            cout<<"Directory deleted successfully"<<endl;
        else
            cout<<"unable to delete a directory"<<endl;
        return ;
    }
    else if(cmd[0]=="rename")
    {
        if(cmd.size()<=2)
        {
            cout<<"INVALID ARGUMENTS"<<endl;
            return ;
        }
        if(rename(cmd))
            cout<<"renaming successfull"<<endl;
        else
            cout<<"renaming not successfull"<<endl;
    }
    else if(cmd[0]=="goto")
    {
        // cout<<cmd[1]<<endl;
        if(cmd.size()<=1)
        {
            cout<<"INVALID ARGUMENTS"<<endl;
            return ;
        }
        // cout<<endl;
        if(Goto(cmd[1]))
        {
            cout<<endl<<"successful"<<endl;
        }
            
        else
            cout<<"failure"<<endl;
    }
    else if(cmd[0]=="search")
    {
        if(cmd.size()<=1)
        {
            cout<<"INVALID ARGUMENTS"<<endl;
            return ;
        }
        string fileName=getFileNameFromPath(cmd[1]);
        if(search(current_working_directory,fileName))
            cout<<"True"<<endl;
        else
            cout<<"False"<<endl;
    }
    else if(cmd[0]=="create_file")
    {
        if(cmd.size()<=2)
        {
            cout<<"INVALID ARGUMENTS"<<endl;
            return ;
        }
        string fileName=getFileNameFromPath(cmd[1]);
        if(createFile(fileName,cmd[2]))
        {
            if(current_working_directory==cmd[2])
            {
                getContents(current_working_directory);
                cout<<endl;
            }
                
            cout<<"file created properly"<<endl;
        } 
        else
            cout<<"file not created properly"<<endl;
    }
    else if(cmd[0]=="create_dir")
    {
        if(cmd.size()<=2)
        {
            cout<<"INVALID ARGUMENTS"<<endl;
            return ;
        }
        string fileName=getFileNameFromPath(cmd[1]);
        if(createDir(fileName,cmd[2]))
        {
            if(current_working_directory==cmd[2])
            {
                getContents(current_working_directory);
                cout<<endl;
            }
                
            cout<<"directory created"<<endl;
        }
        else
            cout<<"directory not created"<<endl;
    }
    else
    {
        cout<<"INVALID COMMAND"<<endl;
        return ;
    }
}
void resize(int dummy)
{
    getWinSize();
    updateCursor();
    printContents();
}
bool commandMode()
{ 
    while(true)
    {
        char c;
        cmd_str="";
        while (read(STDIN_FILENO, &c, 1) == 1)
        {
            if(c==27)
            {
                normalMode=true;
                printContents();
                return true;    
            }
            if(c==127 and cmd_str.size()>0)
            {
                cmd_str.pop_back();
                printContents();
            }
            else if(c=='\n')
            {
                printContents();
                break;
            }
            else if(c!=127)
            {
                cmd_str+=c;
                printContents();
            }
        }
        cout<<endl;
        if(cmd_str=="quit")
        {
            return false;
        }
        vector<string>cmd;
        string word="";
        for(auto each:cmd_str)
        {
            if(each!=' ')
                word+=each;
            else
            {
                if(cmd.size()==0)
                    cmd.push_back(word);
                else
                {
                    cmd.push_back(convert_path_to_absolute(word));
                }
                word="";
            }
        }
        // disableRawMode();
        if(cmd.size()==0)
        {
            cout<<"INVALID number of commands"<<endl;
            return true;
        }
        if(word.size()!=0)
            cmd.push_back(convert_path_to_absolute(word));
        command(cmd);
    }
    return true;
    // cout<<"hello"<<endl;
}
int main()
{
    signal(SIGWINCH,resize);
    getWinSize();
    updateCursor();
    normalMode=true;
   current_working_directory=get_current_dir_name();
   getContents(current_working_directory);
    // getContents("/home/chandu/Desktop/IIITH_SSD");
   //convert terminal from cannonical to non cannonical mode
    enableRawMode();
    // disableRawMode();
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
        switch(c)
        {
            case 'A':
                upperArrow();
                break;
            case 'B':
                lowerArrow();
                break;
            //backspace
            case 127:
                backSpace();
                break;
            case 'D':
                leftArrow();
                break;
            case 'C':
                rightArrow();
                break;
            case 'h':
                home();
                break;
            case '\n':
                enter();
                break;
            case ':':
                normalMode=false;
                printContents();
                if(commandMode()==false)
                    return 1;
                break;
            default:
                break;

        }
    }
    return 0;
}