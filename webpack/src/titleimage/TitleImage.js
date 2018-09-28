import React from 'react'

class TitleImage extends React.Component {
    constructor(props) {
        super(props);
    }

    render() {
        return <img src={this.props.src}></img>
    }
}

export default TitleImage;