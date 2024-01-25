## 核心功能
**核心功能在于发送图像和接收图像，分别对应RemoteCamera和Client。**
### RemoteCamera
1. 要发送图像得有图像，故设计RemoteCamera::record()方法运行一个线程不断拍摄图像。由于此处产生了一个线程，在关闭相机时也要停止该线程，故设计RemoteCamera::_running变量控制线程的停止。
2. 发送图像由RemoteCamera::send_frame()方法完成，发送原始图像由RemoteCamera::download()方法完成。
3. 要解析得到对应指令才能执行RemoteCamera::send_frame()方法或RemoteCamera::download()方法，而要解析指令需要先接收到指令，而要接收指令需要建立起通信才行。在构造函数中建立起通信，由RemoteCamera::receive()方法不断接收和解析指令。
4. 要建立起UDP协议的socket（套接字）通信，需要io_context（上下文管理器）、socket和endpoint（节点，绑定了IP V4地址和端口号）。socket对象的建立需要绑定io_context和endpoint，即socket需要绑定通信结点，交给io_context管理。这一操作在构造函数中完成。
5. io_context管理socket从endpoint发送信息，但接收信息需要从另一默认endpoint处接收，故有RemoteCamera::_sender属性，命名为“_send”是因为接收到的信息几乎都来自于Sender对象。
6. RemoteCamera对象自实例化后，就要不断接收指令，故RemoteCamera::receive()方法直接在构造函数中就被调用。
7. 要不断接收指令，就意味着RemoteCamera::receive()方法要一直运行或不断被调用。一直运行便是方法内有一while(true)循环，但这样会阻塞程序，使得构造函数永远无法执行完，那么RemoteCamera对象永远也不能被实例化，故不能采用一直运行的方式来实现不断接收指令。要采用不断调用的方式来实现不断接收指令，这样构造函数中调用RemoteCamera::receive()方法在执行完后，构造函数也能继续执行，最终实例化RemoteCamera对象。
8. 要不断调用RemoteCamera::receive()方法，需要一个调用者来调用RemoteCamera::receive()方法，这个调用者便是io_context。
9. 那么io_context是如何知道自己要调用RemoteCamera::receive()方法呢？由socket的异步执行方式告诉io_context要调用RemoteCamera::receive()方法。具体过程是socket以异步方式接收或发送信息时，无论是否接收或发送成功，当完成接收或发送后，都会告诉io_context调用一个回调函数（向io_context中注册一个句柄）。当RemoteCamera::receive()方法中的socket以异步方式接收信息，并且交给io_context的回调函数是RemoteCamera::receive()方法本身时，便实现了RemoteCamera::receive()方法的不断调用。
    - 此处所说的发送成功是指将信息发送出去，未报错，而不保证发出的信息被成功接收到；成功接收是指接收到了信息，不报错，但不确定接收的是哪个对象发出的信息，只能知道接收到的信息内容是什么，并且还不保证接收到的信息是完整的。RemoteCamera的这些优缺点是由其采用的UDP协议自身特性决定的，故要深入了解出现这些优缺点的原因便要深入了解UDP协议。RemoteCamera发送信息后不管是否接收到，接收信息时不能区分来源，也不确定接收到的信息是否完整，这体现了UDP协议的无连接和不可靠的特点。要想实现发送信息后能确定被接收，接收信息时能知道来源，也能确保完整接收信息，应采用TCP协议；而RemoteCamera采用UDP协议是为了传输速率，以及每帧数据量小，UDP协议能确保数据完整性；就算数据不完整，图片也只是出现坏点，不会导致整张图像不可识别。同时，由于UDP协议无连接，无需建立会话（session）便可传输信息，这就使得同一个RemoteCamera向单个Client发送图片和向多个Client发送图片的负载是相同的，因为RemoteCamera只管发，不是向特定对象发送，所有Client都能接收到；若采用TCP协议，每多一个Client连接，RemoteCamera就要多一个会话，这就加重了RemoteCamera的负载。
10. 那么io_context又是如何知道自己该在何时调用RemoteCamera::receive()方法呢？io_context会在RemoteCamera::receive()方法执行完毕后，再执行socket交给io_context的回调函数，而这个绘图函数就是RemoteCamera::receive()方法本身，即每次RemoteCamera::receive()方法执行完毕前socket都会告诉io_context要在RemoteCamera::receive()方法执行完毕后再次调用RemoteCamera::receive()方法。
11. 实际的代码中，socket异步方式接收信息的回调函数中还有其他代码，这些代码便是用于解析指令的代码。无论如何解析指令，也无论解析出指令后执行什么操作，最终一定会再次调用RemoteCamera::receive()方法，这样才能确保不断地接收信息。
12. 具体来看RemoteCamera::receive()方法中socket的回调函数，首先要判断error_code是否为false，即判断是否成功接收到信息，只有在成功接收到信息后才能解析指令。最先判断接收到的指令是否为“Next Frame”，因为这一指令是控制传输图像的指令，为了确保Client能流畅显示图像，故RemoteCamera要优先解析“Next Frame”指令。如果不是“Next Frame”，那么判断是否以“od”开头，即判断收到的信息是否为约定的指令格式。若为约定的指令格式，则尝试匹配执行；若无法成功匹配，则说明该指令不是由RemoteCamera执行，那么RemoteCamera会将该指令转发给Client。完成指令解析后，或者根本没有收到信息，都会再次调用RemoteCamera::receive()方法。
13. 那么RemoteCamera是如何转发指令的呢？这一操作仍在RemoteCamera::receive()方法中完成。在收到约定格式的指令并且无法匹配执行时，会将收到的指令转存在RemoteCamera::_message中，在下次接收到“Next Frame”指令后将RemoteCamera::_message中的指令转发给Client。因此在匹配到“Next Frame”指令后并不直接执行RemoteCamera::send_frame()方法，而是判断RemoteCamera::_order_length变量是否大于零，即RemoteCamera::_message中转存的指令长度是否为零，不为零说明确实有转存的指令等待转发。那么将会执行一次异步发送，并将RemoteCamera::_order_length置为零。
14. 之所以要在下一次解析到“Next Frame”指令后才进行转发，是因为只有在此时机进行转发，Client才能正确解析指令。至于为什么Client只有在发出“Next Frame”指令后才能正确解析收到的指令，这需要查看Client的源码，了解Client的工作原理后才能解释，故不在此做详细说明。
15. 在RemoteCamera::send_frame()方法再次调用RemoteCamera::send_frame()方法前，要将RemoteCamera::_order全部置为'\0'，即全部置为字符串截止符。因为只有在socket接收到信息后才会将信息写入RemoteCamera::_order，即只有在接收到新的信息后才会改变RemoteCamera::_order中的内容。若不全置为'\0'，那么下一次未接收到新信息时，RemoteCamera::_order会保留上次接收到的指令，造成重复执行指令，陷入到同一指令的循环中，甚至造成程序错误崩溃。
16. 来看RemoteCamera::send_frame()方法的具体实现。首先对图像进行编码，并且要控制编码的长度不超过89600；若超过了89600，则会降低一档编码质量重新编码，知道编码长度不超过89600。然后会将编码长度发送出去。因为不完整的编码也能解码出不完整的图像，所以必须让Client知道图像编码长度，才能让Client知道图像的完整编码是什么，而不会出现解码不完整，或解码超出实际范围的情况。发送完图像编码长度后，就开始发送图像的编码。首先以1024的步长发送图像编码，最后将余下的不足1024长度的编码一次发送出去。因为socket的默认缓冲区大小是1024，所以发送图像编码的步长设为1024。
17. RemoteCamera::close()方法会将RemoteCamera::_running置为flase、释放摄像头资源、转发“Close”命令、关闭socket的接收和发送，最后停止io_context。其中将RemoteCamera::_running置为flase的操作，会停止拍摄图像的线程。
### Client
1. RemoteCamera和Client分别对应socket通信中的server（服务器）和client（客户端），作为客户端的Client中有resolver（域名解析器）。
2. Client也要有一个socket，同样的，Client的socket也要绑定一个节点，但客户端的节点一般不会使用，都是用服务器的节点通信，故将Client的socket绑定自身IP的零号端口。其中，零号端口是一个预留的端口号，代表的意思就是它在TCP或者UDP网络传输中应该不会被用到。
3. 构造函数中解析器会解析传入的IP地址和端口号，然后返回解析到的若干节点。由于此处是直接解析IP地址和端口，只会得到一个节点；在解析域名（如www.pornhub.com）时，才会得到多个节点。
4. Clinet会将接收到的图像编码存储在Client::_code（std::vector容器）中，图像编码的长度一般超过std::vector的默认长度，故在构造函数中对Client::_code进行预扩容，防止在接收图像编码时自动扩容，造成性能下降。
5. Client要实时显示RemoteCamera发送的图像，就得先不断接受RemoteCamera发送的图像，故Client也有Client::receive()方法。Client::receive()方法和RemoteCamera::receive()一样，在构造函数中就被调用，也会在socket异步接收后调用自身。
6. Client::receive()方法会先发送“Next Frame”，然后再异步接收指令或图像。这就是为什么RemoteCamera要在下一次解析到“Next Frame”指令后才转发指令的原因。
7. Client::receive()方法异步接收到信息后，会先判断信息是否以“od”开头，即判断接收到的信息是否为指令，如果是指令，则匹配执行。若接收到的信息不以“od”开头，也不以“rp”开头，那么认为接收到的信息是由RemoteCamera::send_frame()方法产生的，此时接收到的信息是图像编码的长度，然后调用Client::show()方法。
8. Client::show()方法首先对存储在Client::_cache中的信息进行处理，即对接收到的图像编码长度信息进行处理。图像编码长度是整数，但发送时会转为字符串发送，接收到的也是字符串，要将字符串转为整数。在将字符串形式的图像编码长度信息转为整数前，要在字符串后添加字符串截止符才能确保正确地将字符串转为整数。为了方便，约定图像编码长度信息会是一个5位整数，不会少于5位，也不会超过5位。因为经过测试，即使是低分辨率、低质量的图像的编码长度也是5位整数，故图像编码长度信息不会少于5位；高分辨率或高质量的图像的编码长度可能会达到6位，但最高位也只会是1，即十万长度的编码，经过不降低太多质量的压缩后，编码长度便会降低5位整数。并且实验发现，将十万长度的图像编码压缩到89600长度以下，图像仍有较高清晰度，故RemoteCamera会将图像编码先压缩到98600长度以下再发送，Client会将用于存储图像编码的std::vector容器的长度预扩容至89600。
9. Client::show()方法在获取了图像编码长度后，开始接收图像编码，累加每次接收到的编码长度，直到等于图像编码长度为止。然后解码成图像。
10. Client::show()方法在解码出图像后，先判断是否要标记二维码，若要标记二维码，则会将图像中的二维码用红色四边形框出；然后判断是否要录制视频，即判断是否要将当前帧写入视频文件中，若要录制视频，则会令cv::VideoWriter将当前帧写入视频文件中。最后显示图像，持续30毫秒。
11. Client提供了录制视频的功能，即将接收到的每帧图像写入视频文件中。该功能依赖Client::start_record()和Client::stop_record()两个方法实现。
12. Client::start_record()方法会先接收RemoteCamera发来的帧率和分辨率信息，然后利用这些信息初始化cv::VideoWriter。
13. Client::stop_record()方法会释放cv::VideoWriter资源，并将控制是否录制视频的标记Client::_if_write_video置为false。
14. Client::close()方法会释放cv::VideoWriter资源、删除二维码解码器（cv::QRCodeDetector）、关闭socket的接收和发送，最后停止io_context。
## 扩展功能
**发送命令，控制RemoteCamera和Client执行预定操作**
### Sender
1. Sender与Client一样也是socket通信中的客户端，故Sender的基本结构与Client相同，但因为Sender要发送指令，故多出来Sender::send()方法。
2. Sender::receive()方法仅会在控制台输出接收到的信息，无实质功能，此处留作扩展。
3. Sender::send()方法用于发送指令。首先会接收指令，然后解析指令。由于有命令行和图形界面两种运行模式，这两种模式下接收指令的方式不同，将在GUI化改造部分讲解，在此略过。
4. RemoteCamera工程中约定，所有指令以“od”开头，并且总长度不超过64，即指令有效长度为62。所以Sender::send()方法在接收到指令后要判断长度是否超过64，又因为后续会在开头加上“od”字符，故Sender::send()方法在接收到指令后会判断其长度是否超过62。若指令长度超过62，会清空指令内容。
5. 若指令为空，则会向RemoteCamera发送空指令，然后再次调用Sender::send()方法；若指令不为空，则会在指令开头加上“od”后向RemoteCamera发送，若指令是“Download”，则会调用Download方法，最后再次调用Sender::send()方法。
6. Sender::download()方法与Client::show()方法基本相同，只是前者接收到的图像是未经压缩的，图像编码长，后者接收到的图像是经过压缩的，图像编码短。Sender::download()方法仅接收图像，然后写入磁盘中，不需要显示，也不需要标记二维码和写入视频文件中。
7. Sender主要是向RemoteCamera这个服务器发送信息，而Client主要是从RemoteCamera这个服务器接收信息。
## GUI化改造
### RemoteCamera
1. RemoteCamera始终是以命令行模式运行，不涉及GUI化改造。
### Client
1. 增加Client::_if_gui属性，用于标记是否为GUI模式运行。
2. 以GUI模式运行时，不进行任何控制台输出。
3. 以GUI模式运行时，Client::show()方法在接受图像、完成二维码标记和写入视频文件后直接返回，不由OpenCV显示图像。
### Sender
1. 增加Sender::_if_gui属性，用于标记是否为GUI模式运行。
2. 以命令行模式运行时，Sender::send()从终端获取键盘输入的指令，然后继续执行后续代码。
3. 以GUI模式运行时，指令并不直接获取，而是通过指令代码间接获取。Sender::send()方法会检查Sender::_gui_cmd的值，即检查指令代码，若Sender::_gui_cmd的值在预设的指令代码范围内，则会将指令设置为指令代码所代表的内容；如果没有指令输入，那么Sender::_gui_cmd将会被置于-1，表示无指令。GUI模式下的Sender::send()在获取指令时是非阻塞的，无指令输入时，会发送空指令。
4. Sender提供Sender::get_cmd()方法设置指令代码，该方法被包装在界面的槽函数中。
### ClientGUI
1. 虽然名为“Client”，但实际上同时有Sender和Client。
2. 界面上一些控件绑定有槽函数，参函数会在触发了所绑定的信号时被调用。
3. 首先应输入IP地址和端口。IP地址和端口都有一定的格式或参数要求，如果输入错误，会导致程序错误崩溃，因此要对IP和端口号的输入做出限制。通过正则表达式限制了IP的输入，通过整数验证器（QIntValidator）限制了端口号的输入。但对于端口号的限制，仅限于要求输入的端口号为有效端口号，即0至65535，并未对其安全性做出限制，因此可能会出现与其他程序所用端口冲突，导致系统异常。一般0至1024端口为系统端口，1024至65535为用户端口；其中用户端口又分为BSD临时端口（1024至5000）和BSD服务器（非特权）端口（5001至65535），一般的应用程序使用临时端口来进行通信。
4. 输入IP地址和端口后，点击连接按钮连接RemoteCamera，连接成功便可看到画面。此处“连接”按钮绑定了MainWindow::connect()槽函数，MainWindow::connect()会启动三个线程，这三个线程分别是：MainWindow::run_sender()、MainWindow::run_client()和MainWindow::refresh_graphicsView()。
5. MainWindow::run_sender()方法建立在栈内存中实例化一个io_context，然后在堆内存中实例化一个Sender；MainWindow::run_client()方法建立在栈内存中实例化一个io_context，然后在堆内存中实例化一个Client。
6. MainWindow::refresh_graphicsView()方法在一个循环中不断从MainWindow::_client中读取图像，并将该图像设置为imageLabel（QLabel）的图像，实现实时显示Client接收到的图像。
7. MainWindow::connect()中创建的三个线程都会分离，确保主线程（UI线程）不被阻塞。
# 8. 未完待续。

