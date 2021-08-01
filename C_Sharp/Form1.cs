using System;
using System.Windows.Forms;
using System.Threading;

namespace C_Sharp
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            //取消检查非法的跨线程访问
            CheckForIllegalCrossThreadCalls = false;
            //向列表添加 "main thread: <线程号>"
            listBox1.Items.Add("main thread: " + Thread.CurrentThread.ManagedThreadId.ToString());
        }

        // 向列表添加 "<线程号> 按钮i: <1-5>"
        private void addItem(object o)
        {
            int btnID = (int)o;
            for (int i = 0; i < 5; ++i)
            {
                Thread.Sleep(1000);
                listBox1.Items.Add(Thread.CurrentThread.ManagedThreadId.ToString() + " 按钮" + btnID.ToString() + ": " + (i + 1).ToString());
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Thread t = new Thread(addItem);
            t.IsBackground = true;
            t.Start(1);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            Thread t = new Thread(addItem);
            t.IsBackground = true;
            t.Start(2);
        }
    }
}
