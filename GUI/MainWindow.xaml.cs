///////////////////////////////////////////////////////////////////////
// MainWindow.xaml.cs - GUI for CSE687  Project4                     //
// ver 1.1                                                           //
// Tanming Cui, CSE687 - Object Oriented Design, Spring 2018         //
///////////////////////////////////////////////////////////////////////
/*
 * Package Operations:
 * -------------------
 * This package provides show codes in the server or in the local function
 * Send and receive message from server
 *   
 * Required Files:
 * ---------------
 * Mainwindow.xaml, MainWindow.xaml.cs
 * Translater_TanmingCui.dll
 * 
 * Maintenance History:
 * --------------------
 * ver 1.1 : 10 Apr 2018
 *  - Add basic framwork for remote repository GUI
 * 
 * ver 1.0 : 30 Mar 2018
 * - first release
 * - Several early prototypes were discussed in class. Those are all superceded
 *   by this package.
 */

// Translater has to be statically linked with CommLibWrapper
// - loader can't find Translater.dll dependent CommLibWrapper.dll
using System;
using System.IO;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Input;
using System.Threading;
using System.Text;
using MsgPassingCommunication;
using System.Windows.Documents;

namespace GUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        //----< Instance Object >----------------
        private String[] localpackage;
        private String Description;
        private String Dependency;
        private Translater client;
        private CsEndPoint endPoint_;
        private CsEndPoint serverEndPoint;
        private Thread rcvThrd = null;
        private Dictionary<string, Action<CsMessage>> dispatcher_
      = new Dictionary<string, Action<CsMessage>>();
        private string _path;
        private string _serverpath;
        private List<string> curpath = null;
        private Stack<string> pathStack_ = new Stack<string>();

        //----< Initialise window >----------------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            //serverside
            serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;

            // start Comm
            Random rnd = new Random();
            endPoint_ = new CsEndPoint();
            endPoint_.machineAddress = "localhost";
            endPoint_.port = rnd.Next(8081, 20000);
            client = new Translater();
            client.listen(endPoint_);
            _serverpath = Path.GetFullPath("../../../../ServerFile");
            initializealldispathe();
            test1();
            test2();
            test3();
            test4();
        }

        //----< Initialize all dispatcher >----------------
        private void initializealldispathe()
        {
            localdirsfiles();
            processMessages();
            connecttoserver();
            serverrecvfile();
            finishcheckin();
            finishcheckout();
            loadDispatcher();
            showfulltext();
            checkinfail();
            modifysucc();
            metadatarcv();
            getallchildren();
            failtomodify();
            rcvchildfiles();
            noparentrcv();
        }

        //----< load files in client folder >----------------
        private void localdirsfiles()
        {
            curpath = new List<string>();
            checkinbox.Items.Clear();
            _path = "../../../../ClientFile";
            _path = Path.GetFullPath(_path);
            curpath.Add(_path);
            client.setSendFilePath(_path);
           
            String[] dirs = Directory.GetDirectories(_path);
            localpackage = dirs;
            foreach (String dir in dirs)
            {
                
                checkinbox.Items.Add(Path.GetFileName(dir));
            }
            checkinbox.Items.Insert(0, "..");
            String[] files = System.IO.Directory.GetFiles(_path);
            foreach(String file in files)
            {
                checkinfiles.Items.Add(Path.GetFileName(file));
            }

        }


        //----< process incoming messages on child thread >----------------
        private void processMessages()
        {
            ThreadStart thrdProc = () => {
                while (true)
                {
                    try
                    {
                        CsMessage msg = client.getMessage();
                        string msgId = msg.value("command");
                        if (dispatcher_.ContainsKey(msgId))
                            dispatcher_[msgId].Invoke(msg);
                    }
                    catch(Exception ex)
                    {
                        MessageBox.Show(ex.Message);
                    }
                }
            };
            rcvThrd = new Thread(thrdProc);
            rcvThrd.IsBackground = true;
            rcvThrd.Start();
        }

        //----< add client processing for message with key >---------------
        private void addClientProc(string key, Action<CsMessage> clientProc)
        {
            dispatcher_[key] = clientProc;
        }

        //----< load receive conected in to the dispatch dictionary >---------------
        private void connecttoserver()
        {
            Action<CsMessage> connected = (CsMessage rcvMsg) =>
            {
                if (rcvMsg.value("command") == "connected")
                {
                    Action additem = () =>
                     {
                         connectbox.Items.Add("Successfully connected to server");
                         statusBarText.Text = "Successfully connected";
                     };
                    Dispatcher.Invoke(additem, new Object[] { });
                }
            };
            addClientProc("connected", connected);
        }

        //----< contiunously get children file from repository >---------------
        private void rcvchildfiles()
        {
            Action<CsMessage> allchildfiles = (CsMessage rcvMsg) =>
            {
                if (rcvMsg.value("command") == "childfiles")
                {
                    Action changestatus = () =>
                    {
                        statusBarText.Text = "Get childrenfiles";
                    };
                    Dispatcher.Invoke(changestatus, new Object[] { });
                }

            };
            addClientProc("childfiles", allchildfiles);
        }

        //----< load metadata in to the dispatch dictionary >---------------
        private void metadatarcv()
        {

            Action<CsMessage> metadata = (CsMessage rcvMsg) =>
            {
                if (rcvMsg.value("command") == "metadata")
                {
                    Action metadataget = () =>
                    {
                        statusBarText.Text = "Received metadata";
                    };
                    Dispatcher.Invoke(metadataget, new Object[] { });

                    Action<CsMessage> showmeta = (CsMessage msg) =>
                    {
                        showMetadata(msg);
                    };
                    Dispatcher.Invoke(showmeta, new Object[] { rcvMsg });
                }
            };
            addClientProc("metadata", metadata);
        }

        //----< pop up a window to show file has no parent >---------------
        private void noparentrcv()
        {
            Action<CsMessage> rcvnoparent = (CsMessage rcvMsg) =>
              {
                  if (rcvMsg.value("command") == "noparent")
                  {
                      Action<CsMessage> shownopar = (CsMessage msg) =>
                      {
                          shownoparent(msg);
                      };
                      Dispatcher.Invoke(shownopar, new Object[] { rcvMsg });
                  }
              };
            addClientProc("noparent", rcvnoparent);
        }

        //----< send show metadata request to the repository >---------------
        private void showMetadata(CsMessage msg)
        {
            try
            {
                StringBuilder sb = new StringBuilder();
                sb.AppendLine("File Name: " + msg.value("name"));
                sb.AppendLine("Description: " + msg.value("description"));
                sb.AppendLine("Date Time: " + msg.value("datetime"));
                sb.AppendLine("Dependency: " + msg.value("dependency"));
                sb.AppendLine("Location at repo: " + msg.value("location"));
                sb.AppendLine("Category: " + msg.value("category"));
                sb.AppendLine("isOpen: " + msg.value("isOpen"));
                sb.AppendLine("Version: " + msg.value("version"));
                Paragraph paragraph = new Paragraph();
                paragraph.Inlines.Add(new Run(sb.ToString()));
                CodePopUpWindow popUp = new CodePopUpWindow();
                popUp.Show();
                popUp.CodeView.Blocks.Clear();
                popUp.CodeView.Blocks.Add(paragraph);

            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        //----< send show no parents request >---------------
        private void shownoparent(CsMessage msg)
        {
            try
            {
                StringBuilder sb = new StringBuilder();
                sb.AppendLine("The files in any category that have no parents are");
                sb.AppendLine(msg.value("are"));
                Paragraph paragraph = new Paragraph();
                paragraph.Inlines.Add(new Run(sb.ToString()));
                CodePopUpWindow popUp = new CodePopUpWindow();
                popUp.Show();
                popUp.CodeView.Blocks.Clear();
                popUp.CodeView.Blocks.Add(paragraph);
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        //----< mention client if fail to check in >---------------
        private void checkinfail()
        {
            Action<CsMessage> failtocheckin = (CsMessage rcvMsg) =>
            {
                if (rcvMsg.value("command") == "rounddependency")
                {
                    Action metadataget = () =>
                    {
                        statusBarText.Text = "Fail to checkin";
                    };
                    Dispatcher.Invoke(metadataget, new Object[] { });
                    Action boxshow = () =>
                    {
                        MessageBox.Show("Circular Dependency");
                    };
                    Dispatcher.Invoke(boxshow, new Object[] { });
                }
            };
            addClientProc("rounddependency", failtocheckin);
        }


        //----< load file full text from server >---------------
        private void showfulltext()
        {
            Action<CsMessage> showfulltext = (CsMessage rcvMsg) =>
            {
                if (rcvMsg.value("command") == "filefulltext")
                {
                    try
                    {
                        String filename = rcvMsg.value("sendingFile");
                        filename = Path.GetFileName(filename);
                        filename = Path.Combine(_path, filename);
                        String[] filecontent = File.ReadAllLines(filename);
                        
                        if (rcvMsg.value("content-length") == "0")
                        {
                            Action<String[]> showcontentinwin = (String[] filecontents) =>
                            {
                                showFile(filecontents);
                            };
                            Dispatcher.Invoke(showcontentinwin, new Object[] { filecontent });
                        }
                        File.Delete(filename);
                    }
                    catch(Exception ex)
                    {
                        MessageBox.Show(ex.Message);
                    }
                }
            };
            addClientProc("filefulltext", showfulltext);
        }

        //----< finish check in >---------------
        private void finishcheckin()
        {
            Action<CsMessage> connected = (CsMessage rcvMsg) =>
            {
                if (rcvMsg.value("command") == "finishcheckin")
                {
                    Action additem = () =>
                    {
                        statusBarText.Text = "Successfully Check In";
                    };
                    Dispatcher.Invoke(additem, new Object[] { });
                }
            };
            addClientProc("finishcheckin", connected);
        }

        //----< finish check out >---------------
        private void finishcheckout()
        {
            Action<CsMessage> connected = (CsMessage rcvMsg) =>
            {
                if (rcvMsg.value("command") == "finishcheckout")
                {
                    Action additem = () =>
                    {
                        statusBarText.Text = "Successfully Check out";
                    };
                    Dispatcher.Invoke(additem, new Object[] { });
                }
            };
            addClientProc("finishcheckout", connected);
        }

        //----< load server received file in to the dispatch dictionary >---------------
        private void serverrecvfile()
        {
            Action<CsMessage> filerecved = (CsMessage rcvMsg) =>
            {
                if (rcvMsg.value("command") == "received")
                {
                    Action serverrecved = () =>
                    {
                        statusBarText.Text = "Server received file";
                    };
                    Dispatcher.Invoke(serverrecved, new Object[] { });

                }
            };
            addClientProc("received", filerecved);
        }

        //----< mention client modify success >---------------
        private void modifysucc()
        {
            Action<CsMessage> recvmodify = (CsMessage rcvMsg) =>
            {
                if(rcvMsg.value("command")== "modifysuccess")
                {
                    Action changestatus = () =>
                    {
                        statusBarText.Text = "Successfully change the dependency in the repository";
                    };
                    Dispatcher.Invoke(changestatus, new Object[] { });
                }
            };
            addClientProc("modifysuccess", recvmodify);
        }

        //----< Click connect tab, send connect message >----------------
        private void connect_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                CsMessage msg = new CsMessage();
                msg.add("to", CsEndPoint.toString(serverEndPoint));
                msg.add("from", CsEndPoint.toString(endPoint_));
                msg.add("command", "connect");
                connectbox.Items.Add("Trying to connect to Server");
                client.postMessage(msg);
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        //----< view file content or directories >----------------
        private void checkinbox_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            try
            {
                string content = checkinbox.SelectedItem.ToString();
                checkinbox.Items.Clear();
                checkinfiles.Items.Clear();
                if (content == "..")
                {
                    if (curpath.Count > 1)
                    {
                        curpath.RemoveAt(curpath.Count - 1);
                    }
                }
                else curpath.Add(content);
                string location = _path;
                for (int i = 1; i < curpath.Count; ++i)
                {
                    location = System.IO.Path.Combine(location, curpath[i]);
                }
                String[] dirs = System.IO.Directory.GetDirectories(location);
                String[] files = System.IO.Directory.GetFiles(location);
                foreach (String dir in dirs)
                {
                    checkinbox.Items.Add(System.IO.Path.GetFileName(dir));
                }
                checkinbox.Items.Insert(0, "..");
                foreach (String file in files)
                {
                    checkinfiles.Items.Add(System.IO.Path.GetFileName(file));
                }

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        //----< mention client that fial to modify >---------------
        private void failtomodify()
        {
            Action<CsMessage> fail = (CsMessage rcvMsg) =>
            {
                if (rcvMsg.value("command") == "modifyfail")
                {
                    Action showfail = () =>
                    {
                        MessageBox.Show("Fail to modify");
                    };
                    Dispatcher.Invoke(showfail, new Object[] { });
                }
            };
            addClientProc("modifyfail", fail);
        }

        //----<continuously get all children file from repository >---------------
        private void getallchildren()
        {
            Action<CsMessage> getallchilds = (CsMessage rcvMsg) =>
            {
                var enumer = rcvMsg.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    if (key.Contains("child"))
                    {
                        Action<String> reqchild = (String childs) =>
                         {
                             CsMessage msg = new CsMessage();
                             msg.add("to", CsEndPoint.toString(serverEndPoint));
                             msg.add("from", CsEndPoint.toString(endPoint_));
                             msg.add("command", "getChilds");
                             msg.add("childname", childs);
                             client.postMessage(msg);
                         };
                        Dispatcher.Invoke(reqchild, new Object[] { enumer.Current.Value });
                    }
                }

            };
            addClientProc("haschildren", getallchilds);
        }

        //----< get directories from server >----------------
        private void DispatcherLoadGetDirs()
        {
            Action<CsMessage> getDirs = (CsMessage rcvMsg) =>
            {
                Action clrDirs = () =>
                {
                    checkoutdirs.Items.Clear();
                };
                Dispatcher.Invoke(clrDirs, new Object[] { });
                var enumer = rcvMsg.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    if (key.Contains("dir"))
                    {
                        Action<string> doDir = (string dir) =>
                        {
                            checkoutdirs.Items.Add(dir);
                        };
                        Dispatcher.Invoke(doDir, new Object[] { enumer.Current.Value });
                    }
                }
                Action insertUp = () =>
                {
                    checkoutdirs.Items.Insert(0, "..");
                };
                Dispatcher.Invoke(insertUp, new Object[] { });
            };
            addClientProc("getDirs", getDirs);
        }

        //----< load getFiles processing into dispatcher dictionary >------
        private void DispatcherLoadGetFiles()
        {
            Action<CsMessage> getFiles = (CsMessage rcvMsg) =>
            {
                Action clrFiles = () =>
                {
                    checkoutfiles.Items.Clear();
                };
                Dispatcher.Invoke(clrFiles, new Object[] { });
                var enumer = rcvMsg.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    if (key.Contains("file"))
                    {
                        Action<string> doFile = (string file) =>
                        {
                            checkoutfiles.Items.Add(file);
                        };
                        Dispatcher.Invoke(doFile, new Object[] { enumer.Current.Value });
                    }
                }
            };
            addClientProc("getFiles", getFiles);
        }

        //----< load all dispatcher processing >---------------------------
        private void loadDispatcher()
        {
            DispatcherLoadGetDirs();
            DispatcherLoadGetFiles();
        }

        //----< strip off name of first part of path >---------------------
        private string removeFirstDir(string path)
        {
            string modifiedPath = path;
            int pos = path.IndexOf("/");
            modifiedPath = path.Substring(pos + 1, path.Length - pos - 1);
            return modifiedPath;
        }

        //----< show file full content >----------------
        private void showFile(String[] content)
        {
            try
            {
                StringBuilder sb = new StringBuilder();
                foreach (string con in content) sb.AppendLine(con);
                Paragraph paragraph = new Paragraph();
                paragraph.Inlines.Add(new Run(sb.ToString()));
                CodePopUpWindow popUp = new CodePopUpWindow();
                popUp.Show();
                popUp.CodeView.Blocks.Clear();
                popUp.CodeView.Blocks.Add(paragraph);
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
     

        //----< get directories and files from server >----------------
        private void getdirs_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                pathStack_.Push("../Repository");
                statusBarText.Text = "Repository Files and Directories";
                CsMessage msg = new CsMessage();
                msg.add("to", CsEndPoint.toString(serverEndPoint));
                msg.add("from", CsEndPoint.toString(endPoint_));
                msg.add("command", "getDirs");
                msg.add("path", "../Repository");
                client.postMessage(msg);
                msg.remove("command");
                msg.add("command", "getFiles");
                client.postMessage(msg);
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        //----< send check in request to server >----------------
        private void checkin_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                String content = checkinfiles.SelectedItem.ToString();
                string location = _path;
                String catagories = null;
                for (int i = 1; i < curpath.Count; ++i){
                    if (i < curpath.Count - 1) catagories = catagories + curpath[i] + " ";
                    else if (i == curpath.Count - 1) catagories += curpath[i];
                    location = Path.Combine(location, curpath[i]);
                }
                Description = description.Text;
                Dependency = dependency.Text;
                String[] depends = Dependency.Split(',');
                String depend = null;
                for(int i = 0; i < depends.Length; ++i){
                    if (i < depends.Length - 1) depend = depend + depends[i] + " ";
                    else if (i == depends.Length - 1) depend += depends[i];
                }
                description.Text = "";
                dependency.Text = "";
                CsMessage msg = new CsMessage();
                msg.add("to", CsEndPoint.toString(serverEndPoint));
                msg.add("from", CsEndPoint.toString(endPoint_));
                msg.add("command", "checkin");
                msg.add("description", Description);
                msg.add("dependency", depend);
                msg.add("category", catagories);
                msg.add("sendingFile", Path.Combine(location,content));
                msg.add("destination", _serverpath);
                msg.add("verbose", "haha");
                if (isopen.IsChecked == true) msg.add("isOpen", "open");
                if (isclose.IsChecked == true) msg.add("isOpen", "close");
                client.postMessage(msg);
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        //----< send check out request to server >----------------
        private void checkout_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                String content = checkoutfiles.SelectedItem.ToString();
                CsMessage msg = new CsMessage();
                msg.add("to", CsEndPoint.toString(serverEndPoint));
                msg.add("from", CsEndPoint.toString(endPoint_));
                msg.add("command", "checkout");
                msg.add("key", content);
                client.postMessage(msg);
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        //----< respond to mouse double-click on dir name >----------------
        private void checkoutdirs_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            string selectedDir = (string)checkoutdirs.SelectedItem;
            string path;
            if (selectedDir == "..")
            {
                if (pathStack_.Count > 1)  // don't pop off "Storage"
                    pathStack_.Pop();
                else
                    return;
            }
            else
            {
                path = pathStack_.Peek() + "/" + selectedDir;
                pathStack_.Push(path);
            }
            // display path in Dir TextBlcok
            statusBarText.Text = removeFirstDir(pathStack_.Peek());        
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "getDirs");
            msg.add("path", pathStack_.Peek());
            client.postMessage(msg);
            // build message to get files and post it
            msg.remove("command");
            msg.add("command", "getFiles");
            client.postMessage(msg);
        }

        //----< respond to mouse double-click on server's file >----------------
        private void checkoutfiles_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            try
            {
                string filename = checkoutfiles.SelectedItem.ToString();
                string fileinserv = "../" + statusBarText.Text + "/" + filename;
                CsMessage msg = new CsMessage();
                msg.add("to", CsEndPoint.toString(serverEndPoint));
                msg.add("from", CsEndPoint.toString(endPoint_));
                msg.add("command", "getfilefulltext");
                msg.add("path", fileinserv);
                client.postMessage(msg);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        //----< test case >---------------
        private void test1()
        {
            try
            {
                MouseButtonEventArgs e1 = new MouseButtonEventArgs(Mouse.PrimaryDevice, 0, MouseButton.Left);
                Console.WriteLine("\n  \n   Show GUI connecting to Server");
                connect_Click(this, e1);
                Console.WriteLine("\n  \n   Show GUI disconnect to Server");
                disconnect_Click(this, e1);
                connect_Click(this, e1);
                Console.WriteLine("\n \n   Show Gui send Check-in request");
                Console.WriteLine("\n   Close check in a file has no dependency, and show the version function");
                checkinbox.SelectedIndex = 1;
                checkinbox_MouseDoubleClick(this, e1);
                checkinbox.SelectedIndex = 1;
                checkinbox_MouseDoubleClick(this, e1);
                checkinfiles.SelectedIndex = 0;
                //dependency.Text = "MPCommService.cs";
                description.Text = "Child process of MSbuild";
                isclose.IsChecked = true;
                checkin_Click(this, e1);
                description.Text = "Child process of MSbuild";
                isclose.IsChecked = true;
                checkin_Click(this, e1);
                Console.WriteLine("\n   view the metadata of the latest checked in file");
                CsMessage msg = new CsMessage();
                msg.add("to", CsEndPoint.toString(serverEndPoint));
                msg.add("from", CsEndPoint.toString(endPoint_));
                msg.add("command", "viewmetadata");
                msg.add("key", "Childbuilder.cs.v2");
                client.postMessage(msg);
                Console.WriteLine("\n   view the full content of the latest checked in file");
                CsMessage msg1 = new CsMessage();
                msg1.add("to", CsEndPoint.toString(serverEndPoint));
                msg1.add("from", CsEndPoint.toString(endPoint_));
                msg1.add("command", "getfilefulltext");
                msg1.add("path", "../Repository/CSE681_Project4/Childbuilder/Childbuilder.cs.v2");
                client.postMessage(msg1);
            }
            catch (Exception ex) { Console.WriteLine(ex.Message); }
        }

        //----< test case >---------------
        private void test2()
        {
            Console.WriteLine("\n   Modify the dependency of the file Childbuilder.cs.v2 in repository");
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "modify");
            msg.add("key", "Childbuilder.cs.v2");
            msg.add("dependency", "MPCommService.cs");
            client.postMessage(msg);
            Console.WriteLine("\n   The modification will make the status of the file to open");
            Console.WriteLine("\n   View the metadata of the file");
            CsMessage msg1 = new CsMessage();
            msg1.add("to", CsEndPoint.toString(serverEndPoint));
            msg1.add("from", CsEndPoint.toString(endPoint_));
            msg1.add("command", "viewmetadata");
            msg1.add("key", "Childbuilder.cs.v2");
            client.postMessage(msg1);
            Console.WriteLine("Continue to open check in other file to repository");
            CsMessage msg2 = new CsMessage();
            msg2.add("to", CsEndPoint.toString(serverEndPoint));
            msg2.add("from", CsEndPoint.toString(endPoint_));
            msg2.add("command", "checkin");
            msg2.add("description", "Blocking queue for message list");
            msg2.add("dependency", "");
            msg2.add("category", "CSE681_Project4 MessagePassingCommService");
            msg2.add("sendingFile", Path.GetFullPath("../../../../ClientFile/CSE681_Project4/MessagePassingCommService/BlockingQueue.cs"));
            msg2.add("destination", _serverpath);
            msg2.add("verbose", "haha");
            msg2.add("isOpen", "open");
            client.postMessage(msg2);
            CsMessage msg3 = new CsMessage();
            msg3.add("to", CsEndPoint.toString(serverEndPoint));
            msg3.add("from", CsEndPoint.toString(endPoint_));
            msg3.add("command", "checkin");
            msg3.add("description", "Interface for message comm");
            msg3.add("dependency", "");
            msg3.add("category", "CSE681_Project4 IMessagePassingCommService");
            msg3.add("sendingFile", Path.GetFullPath("../../../../ClientFile/CSE681_Project4/IMessagePassingCommService/IMPCommService.cs"));
            msg3.add("destination", _serverpath);
            msg3.add("verbose", "haha");
            msg3.add("isOpen", "open");
            client.postMessage(msg3);
        }

        //----< test case >---------------
        private void test3()
        {
            CsMessage msg2 = new CsMessage();
            msg2.add("to", CsEndPoint.toString(serverEndPoint));
            msg2.add("from", CsEndPoint.toString(endPoint_));
            msg2.add("command", "checkin");
            msg2.add("description", "Communication channel");
            msg2.add("dependency", "BlockingQueue.cs IMPCommService.cs");
            msg2.add("category", "CSE681_Project4 MessagePassingCommService");
            msg2.add("sendingFile", Path.GetFullPath("../../../../ClientFile/CSE681_Project4/MessagePassingCommService/MPCommService.cs"));
            msg2.add("destination", _serverpath);
            msg2.add("verbose", "haha");
            msg2.add("isOpen", "open");
            client.postMessage(msg2);
            Console.WriteLine("\n   When view the metadata of the Childbuilder.cs.v2, you can see all its required files");
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "viewmetadata");
            msg.add("key", "Childbuilder.cs.v2");
            client.postMessage(msg);
            CsMessage msg3 = new CsMessage();
            msg3.add("to", CsEndPoint.toString(serverEndPoint));
            msg3.add("from", CsEndPoint.toString(endPoint_));
            msg3.add("command", "checkin");
            msg3.add("description", "Main process of the MSbuild");
            msg3.add("dependency", "MPCommService.cs");
            msg3.add("category", "CSE681_Project4 Motherbuilder");
            msg3.add("sendingFile", Path.GetFullPath("../../../../ClientFile/CSE681_Project4/Motherbuilder/Motherbuilder.cs"));
            msg3.add("destination", _serverpath);
            msg3.add("verbose", "haha");
            msg3.add("isOpen", "open");
            client.postMessage(msg3);
            Console.WriteLine("Display all of the files in any category that have no parents");
            CsMessage msg1 = new CsMessage();
            msg1.add("to", CsEndPoint.toString(serverEndPoint));
            msg1.add("from", CsEndPoint.toString(endPoint_));
            msg1.add("command", "noparent");
            client.postMessage(msg1);
        }

        //----< test case >---------------
        private void test4()
        {
            Console.WriteLine("\n   Modify the dependency of a file in repository, try a circle dependency");
            CsMessage msg = new CsMessage();
            msg.add("to", CsEndPoint.toString(serverEndPoint));
            msg.add("from", CsEndPoint.toString(endPoint_));
            msg.add("command", "modify");
            msg.add("key", "IMPCommService.cs.v1");
            msg.add("dependency", "Childbuilder.cs");
            client.postMessage(msg);
            Console.WriteLine("\n   Try to close check in the Motherbuilder.cs, although all its required files are exisited in repository," +
                "but they are not all closed, so the status of the check in file still be open");
            CsMessage msg3 = new CsMessage();
            msg3.add("to", CsEndPoint.toString(serverEndPoint));
            msg3.add("from", CsEndPoint.toString(endPoint_));
            msg3.add("command", "checkin");
            msg3.add("description", "Main process of the MSbuild");
            msg3.add("dependency", "MPCommService.cs");
            msg3.add("category", "CSE681_Project4 Motherbuilder");
            msg3.add("sendingFile", Path.GetFullPath("../../../../ClientFile/CSE681_Project4/Motherbuilder/Motherbuilder.cs"));
            msg3.add("destination", _serverpath);
            msg3.add("verbose", "haha");
            msg3.add("isOpen", "close");
            client.postMessage(msg3);
            CsMessage msg1 = new CsMessage();
            msg1.add("to", CsEndPoint.toString(serverEndPoint));
            msg1.add("from", CsEndPoint.toString(endPoint_));
            msg1.add("command", "viewmetadata");
            msg1.add("key", "Motherbuilder.cs.v1");
            client.postMessage(msg1);
            Console.WriteLine("\n   Checkout the file Childbuilder.cs.v2 and all its required file to ClientFile folder");
            CsMessage msg4 = new CsMessage();
            msg4.add("to", CsEndPoint.toString(serverEndPoint));
            msg4.add("from", CsEndPoint.toString(endPoint_));
            msg4.add("command", "checkout");
            msg4.add("key", "Childbuilder.cs.v2");
            client.postMessage(msg4);
        }


        //----< view metadata function >----------------
        private void view_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                string filename = checkoutfiles.SelectedItem.ToString();
                CsMessage msg = new CsMessage();
                msg.add("to", CsEndPoint.toString(serverEndPoint));
                msg.add("from", CsEndPoint.toString(endPoint_));
                msg.add("command", "viewmetadata");
                msg.add("key", filename);
                client.postMessage(msg);
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        //----< mock disconnect, generate a new port to communicate with server >---------------
        private void disconnect_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                //serverside
                serverEndPoint = new CsEndPoint();
                serverEndPoint.machineAddress = "localhost";
                serverEndPoint.port = 8080;

                // start Comm
                Random rnd = new Random();
                endPoint_ = new CsEndPoint();
                endPoint_.machineAddress = "localhost";
                endPoint_.port = rnd.Next(8081, 20000);
                client = new Translater();
                client.listen(endPoint_);
                processMessages();
                connectbox.Items.Clear();
                connectbox.Items.Add("Successfully disconnect from server");
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        //----< generate modify request >---------------
        private void modify_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                string filename = checkoutfiles.SelectedItem.ToString();
                CsMessage msg = new CsMessage();
                msg.add("to", CsEndPoint.toString(serverEndPoint));
                msg.add("from", CsEndPoint.toString(endPoint_));
                msg.add("command", "modify");
                msg.add("key", filename);
                String changedepends = changedepend.Text;
                changedepend.Text = "";
                String[] changedep = changedepends.Split(',');
                String depend = null;
                for (int i = 0; i < changedep.Length; ++i)
                {
                    if (i < changedep.Length - 1) depend = depend + changedep[i] + " ";
                    else if (i == changedep.Length - 1) depend += changedep[i];
                }
                msg.add("dependency", depend);
                client.postMessage(msg);

            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        //----< generate get noparent file list request >---------------
        private void noparent_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                CsMessage msg = new CsMessage();
                msg.add("to", CsEndPoint.toString(serverEndPoint));
                msg.add("from", CsEndPoint.toString(endPoint_));
                msg.add("command", "noparent");
                client.postMessage(msg);
            }
            catch(Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
    }
}
