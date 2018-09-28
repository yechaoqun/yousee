
import React from 'react'
import styled from 'styled-components'


class SideBarLabel extends React.Component {
    constructor(props) {
        super(props);
        this.openlabel=false;
        this.state={openlabel:false};
    }

    render() {
        return this.creatLable(this.props.active, this.props.label);
    }

    creatLable(highlight=false, label) {
        const A=styled.a.attrs({active: highlight})`
            display: block;
            width: 130px;
            padding: 0 0 0 10px;
            padding-top: 0px;
            padding-right: 0px;
            padding-bottom: 0px;
            padding-left: 10px;
            color: ${props => props.active ? "#1cd388": "#b2b4c4"};
            font-size: 18px;
            line-height: 40px;
            transition: all .3s ease-out;
            transition-property: all;
            transition-duration: 0.3s;
            transition-timing-function: ease-out;
            transition-delay: 0s;
            text-decoration: none;
        `;
        const I=styled.i`
            position: absolute;
            right: 0;
            top: 0;
            color: #b2b4c4;
            font-size: 24px;
            line-height: 40px;
            transition: all .3s ease-out;
            cursor: pointer;
            :hover {
                border-color: #FFFFFF
            }
        `;
        const RightI=I.extend`
            display: ${props => props.openlabel ? "none" : "block"};
            position: absolute;
            top: 17px;
            width: 6px;
            height: 6px;
            border-top: 2px solid #b2b4c4;
            border-right: 2px solid #b2b4c4;
            transform: rotate(45deg);
        `;
        const DownI=I.extend`
            display: ${props => props.openlabel ? "block" : "none"};
            position: absolute;
            top: 17px;
            width: 6px;
            height: 6px;
            border-top: 2px solid #b2b4c4;
            border-right: 2px solid #b2b4c4;
            transform: rotate(135deg);
        `;
        return (
            <div>
                {
                    this.creatHref(A, label.href, label.text)
                }
                <RightI onClick={this.openLable.bind(this)} openlabel={this.state.openlabel}></RightI>
                <DownI onClick={this.openLable.bind(this)} openlabel={this.state.openlabel}></DownI>
                {
                    
                    this.creatSubLabel(label.sublabel)
                }
            </div>
        );
    }

    creatSubLabel(sublabel) {
        let list=(A, sublabel) => {
            let res=[];
            for(let i=0; i<sublabel.length; i++)
                res.push(this.creatHref(A, sublabel[i].href, sublabel[i].text));
            return res;
        }
        const Div=styled.div.attrs({openlabel:this.state.openlabel})`
            display: ${props => props.openlabel ? "block" : "none"};
            position: relative;
            margin: 0 0 0 -24px;
            width: 155px;
            overflow: hidden;
            padding: 4px 9px 4px 14px;
            background-color: #2d2c3e;
            border-radius: 2px;
        `;
        const A=styled.a`
            display: "block";
            float: left;
            min-width: 72px;
            max-width: 100%;
            margin-right: 5px;
            line-height: 34px;
            font-size: 12px;
            color: #b2b4c4;
            overflow: hidden;
            height: 34px;
            text-overflow: ellipsis;
            white-space: nowrap;
            transition: all .3s ease-out;
            text-decoration: none;
        `;
        
        if(sublabel.length>0)
            return (<Div>{list(A, sublabel)}</Div>);
    }

    creatHref(A, href, text) {
        const Ahover=styled(A)`
            :hover {
                color: #FFFFFF
            }
        `;
        return (
            <Ahover href={href}>{text}</Ahover>
        );
    }

    openLable() {
        if(this.openlabel==false) {
            this.openlabel=true;
            this.setState({openlabel:true});
        }
        else {
            this.openlabel=false;
            this.setState({openlabel:false});
        }
    }
}

class SideBar extends React.Component {
    constructor(props) {
        super(props);
        
    }

    render() {
        return this.creatBar(this.props.config);
    }

    /*创建一个分类标签*/
    creatBar(config) {
        let list=(config) => {
            let res=[];
            for(let i=0; i<config.labels.length; i++) {
                console.log(`${config.privatedata} ${config.labels[i].tag}`);
                res.push(this.creatLable(config.privatedata==config.labels[i].tag, config.labels[i]));
            }
            return res;
        }

        const Ul=styled.ul`
            position: relative;
            list-style-type: none;
        `;
        
        return (
            <Ul>
                { 
                    list(config)
                }
            </Ul>
        );
    }

    creatLable(active=false, label) {
        const Li=styled.li`
            position: relative;
            list-style-type: none;
            width: 155px;
        `;
        return (
            <Li>
                <SideBarLabel active={active} label={label}>
                </SideBarLabel>
            </Li>
        );
    }
}

export default SideBar;