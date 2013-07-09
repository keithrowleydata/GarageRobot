Imports Microsoft.VisualBasic
Imports System.IO.Ports

Public Class GarageDoor
    Private s As New SerialPort

    Private Function RunCommand(ByVal Command As String) As String
        Dim Message As String

        Try
            s.Close()
            s.PortName = "COM1"  'will need to change to your port number
            s.BaudRate = 9600
            s.DataBits = 8
            s.Parity = Parity.None
            s.StopBits = StopBits.One
            s.Handshake = Handshake.None
            s.Encoding = System.Text.Encoding.ASCII 'very important!
            s.RtsEnable = False

            s.Open()

            s.WriteLine(Command)
            Message = s.ReadLine
            s.Close()

        Catch ex As Exception
            s.Close()

            Return ex.ToString
        End Try

        Return Message
    End Function

    Public Function GetStatus() As String
        Return RunCommand("g")
    End Function

    Public Function ClickGarageDoor() As String
        Return RunCommand("c")
    End Function
End Class
